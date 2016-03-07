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
//const unsigned long IO_MEMORY = (long)2 * (long)1024 * (long)1024 * 1024;
const long APPROX_LINE_LENS=45;
//const long STD_THRESHOLD=3;

//const long MAX_BYTES = sizeof(char)*MAX_LINES*45; //each line is around 41 characters long
//

void process_data(vector<Tick> &tick_data, RunningStat *pxstats, RunningStat *mRstats, vector<Tick *> &bad_ticks,
                  float std_threshold);

int main (int argc, char *argv[])
{
    // MPI initialization, assign rank and size value

	MPI_Status mpi_status;
	MPI_Init(&argc, &argv);

	int mpi_rank, mpi_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    // read all parameters
    Parameters params; // logging parameters
    string fname_input,fname_noise,fname_data; // file names
    unsigned long IO_MEMORY;
    float STD_THRESHOLD;

    string params_fname=argv[1];
    ifstream file(const_cast<char*>(params_fname.c_str()));
    string line;
    while (getline(file,line))
    {
        vector<string>datas=parse_string(line,' ');
        if (datas.empty()) continue;
        if (datas[0]=="-data")
            fname_data=datas[1];
        else if (datas[0]=="-noise")
            fname_noise=datas[1];
        else if (datas[0]=="-input")
            fname_input =datas[1];
        else if (datas[0]=="-io_memory")
            IO_MEMORY=atoll(datas[1].c_str());
        else if (datas[0]=="-price_std")
            STD_THRESHOLD=atof(datas[1].c_str());
    }
    file.close();
	// maximum memory each process can use
	const unsigned long MAX_BYTES= IO_MEMORY / mpi_size/ sizeof(char);

	// open data file
	MPI_File in_data,out_noise;
	int ierr=MPI_File_open(MPI_COMM_WORLD, const_cast<char*>(fname_input.c_str()), MPI_MODE_RDONLY, MPI_INFO_NULL, &in_data);
	if (ierr){
		if (mpi_rank==0) cout << "Couldn't open file " << fname_input;
		MPI_Finalize();
		exit(0);
	}
    //open noise file
    int oerr = MPI_File_open(MPI_COMM_WORLD,const_cast<char*>(fname_noise.c_str()),MPI_MODE_CREATE | MPI_MODE_WRONLY,MPI_INFO_NULL,&out_noise);
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
    RunningStat pxstats,min_ret_stats;

    //total number of bad ticks
    unsigned long n_bad_ticks=0, n_good_ticks=0;

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
        process_data(data, &pxstats, &min_ret_stats, bad_ticks, STD_THRESHOLD);
        n_bad_ticks+=bad_ticks.size();
        n_good_ticks+=pxstats.NumDataValues();
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
    LOG(params,DIAGNOSTIC,MAIN,to_string(mpi_rank)," num of good ticks: ",to_string(n_good_ticks));
    LOG(params, DIAGNOSTIC, MAIN, "mean: ", to_string(pxstats.Mean()), " std: ", to_string(pxstats.StandardDeviation()));
    LOG(params, DIAGNOSTIC, MAIN, "skewness: ", to_string(min_ret_stats.Skewness()), " kurtosis: ", to_string(min_ret_stats.Kurtosis()));
    double excess_kurtosis= min_ret_stats.Kurtosis();//if this very different from zero - distribution is not normal
    unsigned long n_min_samples=min_ret_stats.NumDataValues();
    LOG(params,DIAGNOSTIC,MAIN,"# of 1 min smaples: ",to_string(n_min_samples));
    // calculate max time for each operation
    double m_read_t,m_parse_t,m_compute_t,m_write_t,a_kurtosis;
    unsigned long t_n_bad_ticks,t_n_good_ticks,t_n_min_samples;
    MPI_Reduce(&read_t.d_time,&m_read_t,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(&parse_t.d_time,&m_parse_t,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(&compute_t.d_time,&m_compute_t,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(&write_t.d_time,&m_write_t,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(&n_bad_ticks,&t_n_bad_ticks,1,MPI_UNSIGNED_LONG,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&excess_kurtosis,&a_kurtosis,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&n_min_samples,&t_n_min_samples,1,MPI_UNSIGNED_LONG,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&n_good_ticks,&t_n_good_ticks,1,MPI_UNSIGNED_LONG,MPI_SUM,0,MPI_COMM_WORLD);
    a_kurtosis/=mpi_size;

    // output to log
    if (mpi_rank==0){
        LOG(params,RESULT,MAIN,"Read time: ",to_string(m_read_t));
        LOG(params,RESULT,MAIN,"Parse time: ",to_string(m_parse_t));
        LOG(params,RESULT,MAIN,"Compute time: ",to_string(m_compute_t));
        LOG(params,RESULT,MAIN,"Write time: ",to_string(m_write_t));
        LOG(params,RESULT,MAIN,"Total # of good ticks: ",to_string(t_n_good_ticks));
        LOG(params,RESULT,MAIN,"Total # of bad ticks: ",to_string(t_n_bad_ticks));
        // since each mpi process looks at approximately the same amount of data,
        // we can simply average the numbers for running statistics like kurtosis (because delta~0)
        // using approximate upper bound for sample kurtosis of 24/n (from https://en.wikipedia.org/wiki/Kurtosis#Sample_kurtosis)
        LOG(params,RESULT,MAIN,"Excess Kurtosis: ",to_string(a_kurtosis),", # of 1min samples: ",to_string(t_n_min_samples));
        string normality_verdict;
        a_kurtosis>24/(t_n_min_samples/mpi_size) ? normality_verdict="Distribution of 1 min returns is NOT normal" : "Distribution of 1 min returns is normal";
        LOG(params,RESULT,MAIN,normality_verdict);
    }

	MPI_Finalize();

	return 0;
}

void process_data(vector<Tick> &tick_data, RunningStat *pxstats, RunningStat *mRstats, vector<Tick *> &bad_ticks,
                  float std_threshold) {
    pxstats->Clear(); // since we don't look at consecutive data chunks, price after gaps can be very different
    sort(tick_data.begin(),tick_data.end());
    //stats for sampling - we going to sample approximately every minute by just taking the first tick when minute value changes
    //it's not precisely a minute, but with a lot of observations those impresicions won't matter much
    // we only need it to determine normality
    const Tick * prior_sample_tick=NULL; // i don't want to accidentally change the prior tick since it already marked as GOOD
    // burn-in stage - push 10 first GOOD ticks just to initialize RunningStats counter
    int pos=0;
    do{
        if (tick_data[pos].status==GOOD ){
            pxstats->Push(tick_data[pos].price );
            prior_sample_tick=&tick_data[pos]; // starting sampling point
        }
        ++pos;
    } while (pxstats->NumDataValues() < 10);
    // filtering against running online variance starts here
    for (unsigned long t = pos; t < tick_data.size(); ++t) {
//        cout<<"price: "<<tick_data[t].price<<" mean px "<<pxstats->Mean()<< " std "<<pxstats->StandardDeviation()<<endl;
        if (tick_data[t].status==GOOD && fabs(tick_data[t].price-pxstats->Mean())/pxstats->StandardDeviation()<=std_threshold) {
            // tick is good, add it to running stats
            pxstats->Push(tick_data[t].price);
            // check if you need to updates return sampling
            // below routine doesn't take care of rare cases when date changes, but hour and minute happen to be the same. Saw that in sample files, should
            // not happen in big files or real world
            int min_change=tick_data[t].minute-prior_sample_tick->minute;
            if (min_change==0){
                ; // ticks are from the same minute - no updates nor sampling is needed
            }
//            else if (min_change==1 || (min_change==-59 && tick_data[t].hour-prior_sample_tick->hour==1)){
            else if (min_change==1 ){
                // we have a one minute change. Sample 1min return and update pointer to prior minute
                mRstats->Push(tick_data[t].price/prior_sample_tick->price-1);
//                cout<<"New Sample. Prior tick: "<<prior_sample_tick->toString()<<endl;
//                cout<<"New tick: "<<tick_data[t].toString()<<endl;
                prior_sample_tick=&tick_data[t];
            }
            else{
                // the change is not zero, and not 1 min - we must have hit a gap. Reset prior tick
//                cout<<"Hit gap. Prior tick: "<<prior_sample_tick->toString()<<endl;
//                cout<<"New tick: "<<tick_data[t].toString()<<endl;
                prior_sample_tick=&tick_data[t];
            }
        }
        else
        {
            // mark tick as bad
            tick_data[t].status=BAD;
//            cout<<"++++adding to bad_ticks "<<tick_data[t].toString()<<endl;
            bad_ticks.push_back(&tick_data[t]);
        }
    }
}
