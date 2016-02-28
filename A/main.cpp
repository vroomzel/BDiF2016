//
// Created by alex on 2/11/16.
//

# include <cstdlib>
# include "mpi.h"

// my includes
#include "mpi_io.h"
#include "Parameters.h"
#include "LOG.h"

using namespace std;

// io parameters
//const long IO_MEMORY = 15 * 0.3 * 1024 * 1024 * 1024; //Penzias has about 15Gb of free RAM, use third of it for reading
// then at most another third will be used for writing signal/noise and about a third will be left for processing
const long IO_MEMORY = 500 * 1024 * 1024;

//const long MAX_BYTES = sizeof(char)*MAX_LINES*45; //each line is around 41 characters long
//
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

	//process data
	for (long cycle=0;cycle<num_cycles;cycle++){
		MPI_Offset cycle_start=cycle*optimal_buffer_size*mpi_size;

		// read data

		read_data(in_data, mpi_rank, mpi_size, mpi_status, cycle_start, optimal_buffer_size, overlap, &ptr_read,&data_start,&data_end);
        LOG(params,CALLED,MAIN,"read_data() has been called");

        vector<Tick> data;
		// parse data
        parse_data(&data_start,&data_end,&data);
        LOG(params,CALLED,MAIN,"parse_data() has been called");
//        for (Tick tick:data){
//            cout<<tick.toString()<<endl;
//        }

		// scrub data


		// write data

//        cout<<"start is "<<data_start[0];
//        cout<<"End is "<<data_end[0]<<" or "<<data_start[strlen(data_start)];
//        cout<<"Length of buffer "<<strlen(data_start);


        oerr=MPI_File_write_ordered(out_noise,data_start,strlen(data_start),MPI_CHAR,&mpi_status);
        if (oerr){
            if (mpi_rank==0) cout<<"Couldn't write to noise.txt. Err code: "<<oerr ;
            MPI_Finalize();
            exit(0);
        }

	}

	MPI_Finalize();

	return 0;
}
