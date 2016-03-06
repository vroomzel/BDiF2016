//
// Created by alex on 2/11/16.
//

#include <cstdlib>
#include "mpi.h"
#include <algorithm>
#include <math.h>

// my includes
#include "mpi_io.h"
#include "Parameters.h"
#include "LOG.h"
#include "Timer.h"
#include "RunningStat.h"

using namespace std;

// io parameters
//const long IO_MEMORY = 15 * 0.3 * 1024 * 1024 * 1024; //Penzias has about 15Gb of free RAM, use third of it for reading
// then at most another third will be used for writing signal/noise and about a third will be left for processing
const long IO_MEMORY = 500 * 1024 * 1024;
const long APPROX_LINE_LENS=45;

//const long MAX_BYTES = sizeof(char)*MAX_LINES*45; //each line is around 41 characters long
//

void process_data(vector<Tick> &tick_data, RunningStat *retstat, vector<Tick *> &bad_ticks);

int main (int argc, char *argv[])
{
	// MPI initialization, assign rank and size value

	MPI_Status mpi_status;
	MPI_Init(&argc, &argv);

	int mpi_rank, mpi_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

	// get file name
	string fname_data = argv[1];

	// read all parameters
	Parameters params;

	// maximum memory each process can use
	const long MAX_BYTES= IO_MEMORY / mpi_size/ sizeof(char);

	// open data file
	MPI_File in_data,out_noise;
	int ierr=MPI_File_open(MPI_COMM_WORLD, const_cast<char*>(fname_data.c_str()),MPI_MODE_RDONLY,MPI_INFO_NULL,&in_data);
	if (ierr){
		if (mpi_rank==0) cout<<"Couldn't open file " << fname_data ;
		MPI_Finalize();
		exit(0);
	}
    //open noise file
    int oerr = MPI_File_open(MPI_COMM_WORLD,"/home/alex/hgdev/gits/BDiF2016/A/data/noise.txt",MPI_MODE_CREATE | MPI_MODE_WRONLY,MPI_INFO_NULL,&out_noise);
    if (oerr){
        if (mpi_rank==0) cout<<"Couldn't open file noise.txt. Error code "<<oerr ;
        MPI_Finalize();
        exit(0);
    }
	// figure out how many cycles you'll have to do to process entire file and what's the optimal buffer size to split
	// the work approximately evenly between processes and cycles

	MPI_Offset filesize;
	MPI_File_get_size(in_data,&filesize);
	MPI_Offset est_bytes_per_process = filesize/mpi_size+1;
	MPI_Offset int_part_of_num_of_max_buffers =est_bytes_per_process/MAX_BYTES;
	MPI_Offset num_cycles = int_part_of_num_of_max_buffers+1;
	MPI_Offset optimal_buffer_size= est_bytes_per_process/(num_cycles)+1;
    MPI_Offset overlap=100;

	if (mpi_rank==0){
		// output some debug info
		cout<<"Total Memory " << IO_MEMORY<<endl;
		cout<<"Max Bytes per process "<<MAX_BYTES<<endl;
		cout<<"File size "<<filesize<<endl;
		cout<<"Estimated bytes per process "<< est_bytes_per_process<<endl;
		cout<<"int_part_of_num_of_max_buffers "<< int_part_of_num_of_max_buffers<<endl;
		cout<<"num cycles "<<num_cycles<<endl;
		cout<<"optimal_buffer_size "<<optimal_buffer_size<<endl;
	}
	// allocate memory for raw data
	char *ptr_read=(char*) malloc((optimal_buffer_size+overlap+1)*sizeof(char)); // +1 to keep '\0' character to define where we ended

	char *data_start, *data_end;

    // Timers
    Timer read_t, parse_t, compute_t, write_t;

    // statistics calculators
    RunningStat pxstats,retstats;

    //total number of bad ticks
    unsigned long n_bad_ticks=0;

	//process data
	for (long cycle=0;cycle<num_cycles;cycle++){
		MPI_Offset cycle_start=cycle*optimal_buffer_size*mpi_size;

		// read data
        read_t.start();
		read_data(in_data, mpi_rank, mpi_size, mpi_status, cycle_start, optimal_buffer_size, overlap, &ptr_read,&data_start,&data_end);
        read_t.end();
        LOG(params,CALLED,MAIN,"read_data() has been called");

        vector<Tick> data;
        vector<Tick *> bad_ticks;
		// parse data
        parse_t.start();
        parse_data(&data_start,&data_end,&data);
        parse_t.end();
        LOG(params,CALLED,MAIN,"parse_data() has been called");

		// scrub data
        compute_t.start();
        process_data(data, &retstats, bad_ticks);
        n_bad_ticks+=bad_ticks.size();
        LOG(params,CALLED,MAIN,"process_data() has been called");
        compute_t.end();

//        LOG(params,DIAGNOSTIC,MAIN,"outputing bad ticks vector");
//        for (size_t t =0; t<bad_ticks.size();++t){
//            cout<<bad_ticks[t]->toString()<<endl;
//        }

		// write data
        LOG(params,DIAGNOSTIC,MAIN,"starting to write data");
        write_t.start();
        // prepare buffer with data
        unsigned long size = (APPROX_LINE_LENS * bad_ticks.size() + overlap) * sizeof(char);
        char *ptr_write=(char*) malloc(size); // +1 to keep '\0' character to define where we ended
        size_t write_length=0;
        for (size_t t =0; t<bad_ticks.size();++t){
            size_t line_len= bad_ticks[t]->end - bad_ticks[t]->beg;
            strncpy(ptr_write+write_length, bad_ticks[t]->beg, line_len);
            write_length+=line_len;
        }
        oerr=MPI_File_write_ordered(out_noise,ptr_write,write_length,MPI_CHAR,&mpi_status);
        free(ptr_write);
        write_t.end();
        LOG(params,DIAGNOSTIC,MAIN,"finished writing data");
        if (oerr){
            if (mpi_rank==0) cout<<"Couldn't write to noise.txt. Err code: "<<oerr ;
            MPI_Finalize();
            exit(0);
        }
	}
    LOG(params,DIAGNOSTIC,MAIN,to_string(mpi_rank)," num of good returns: ",to_string(retstats.NumDataValues()));
    LOG(params,DIAGNOSTIC,MAIN,"mean: ",to_string(retstats.Mean()));
    LOG(params,DIAGNOSTIC,MAIN,"std: ",to_string(retstats.StandardDeviation()));
    // calculate max time for each operation
    double m_read_t,m_parse_t,m_compute_t,m_write_t;
    unsigned long t_n_bad_ticks;
    MPI_Reduce(&read_t.d_time,&m_read_t,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(&parse_t.d_time,&m_parse_t,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(&compute_t.d_time,&m_compute_t,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(&write_t.d_time,&m_write_t,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(&n_bad_ticks,&t_n_bad_ticks,1,MPI_UNSIGNED_LONG,MPI_SUM,0,MPI_COMM_WORLD);

    // output to log
    if (mpi_rank==0){
        LOG(params,RESULT,MAIN,"Read time: ",to_string(m_read_t));
        LOG(params,RESULT,MAIN,"Parse time: ",to_string(m_parse_t));
        LOG(params,RESULT,MAIN,"Compute time: ",to_string(m_compute_t));
        LOG(params,RESULT,MAIN,"Write time: ",to_string(m_write_t));
        LOG(params,RESULT,MAIN,"Total # of bad ticks: ",to_string(t_n_bad_ticks));
    }

	MPI_Finalize();

	return 0;
}

void process_data(vector<Tick> &tick_data, RunningStat *retstat, vector<Tick *> &bad_ticks) {
    sort(tick_data.begin(),tick_data.end());
    // burn-in stage - push 10 first GOOD ticks just to initialize RunningStats counter
    int pos=0;
    do{
        ++pos;
        if (tick_data[pos].status==GOOD && tick_data[pos-1].status==GOOD){
            retstat->Push((tick_data[pos].price/tick_data[pos-1].price-1));
        }
    } while (retstat->NumDataValues()<10);
    // filtering against running online variance starts here
    for (unsigned long t = pos+1; t < tick_data.size(); ++t) {
//        cout<<"price: "<<tick_data[t].price<<" mean ret "<<retstat->Mean()<< " std "<<retstat->StandardDeviation()<<endl;
        if (tick_data[t].status==GOOD) {
            // tick is good based on its time. Check if price within the range
            double ret = tick_data[t].price/tick_data[t-1].price-1;
            if (fabs((ret-retstat->Mean())/retstat->StandardDeviation())<=2.13){
                // tick is good - update online stats
                retstat->Push(ret);
            }
            else{
                // mark tick as bad
                tick_data[t].status=BAD;
//                cout<<"++++addit to bad_ticks "<<tick_data[t].toString()<<" ret: "<<ret<<endl;
                bad_ticks.push_back(&tick_data[t]);
            }
        }
        else
        {
            // mark tick as bad
            tick_data[t].status=BAD;
//            cout<<"++++addit to bad_ticks "<<tick_data[t].toString()<<endl;
            bad_ticks.push_back(&tick_data[t]);
        }
    }
}
