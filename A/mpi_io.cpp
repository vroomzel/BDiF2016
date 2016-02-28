//
// Created by alex on 2/20/16.
//

#include "mpi_io.h"


void read_data(MPI_File &in, const int rank, const int size, MPI_Status status, MPI_Offset &cycle_start,
               MPI_Offset &optimal_buffer_size, const int &overlap, char **chunk, char **real_start,char **real_end)
{
    MPI_Offset filesize;
    MPI_Offset localsize;
    MPI_Offset start;
    MPI_Offset end;

    /* figure out who reads what */
    MPI_File_get_size(in, &filesize);
    start = cycle_start + optimal_buffer_size * rank;
    end = start + optimal_buffer_size - 1;

    /* add overlap to the end of everyone's chunk... */
    end += overlap;

    /* when at the end of file */
    if (end > filesize) end = filesize;

    localsize = end - start+1;
//    cout<<rank<<": "<<start<<" "<<end<<" "<<localsize<<endl;
    /* everyone reads in their part */
    MPI_File_read_at_all(in,start,*chunk,localsize,MPI_CHAR,&status);
    (*chunk)[localsize]='\0';

    /*
     * everyone calculate what their start and end *really* are by going
     * from the first newline after start to the first newline after the
     * overlap region starts (eg, after end - overlap + 1)
     */

    MPI_Offset locstart = 0, locend = localsize;
    if (start != 0) {
        while((*chunk)[locstart] != '\n') ++locstart;
        ++locstart;
    }
    if (end != filesize) {
        locend -= overlap;
        while((*chunk)[locend] != '\n') ++locend;
    }
    localsize = locend - locstart + 1;

    /* point to the real chunk I want to read from */
    *real_start = &((*chunk)[locstart]);
    (*real_start)[localsize] = '\0'; // or should it be '\0' ??
    *real_end=&((*real_start)[localsize]);
//    cout<<*real_start; // for debugging

}

void parse_data(char *const *start, char *const *end, vector<Tick> *tick_data) {
    char const *s = *start; // i don't want to modify original start and end locations
    char const *e = strchr(*start,'\n');
    while ((e!=NULL) & (e<=*end)){
        tick_data->push_back(Tick(s,e));
        s=e+1;
        e=strchr(s,'\n');
    }



}
