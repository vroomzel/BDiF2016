//
// Created by alex on 2/20/16.
//

#ifndef BDIF2016_MPI_IO_H
#define BDIF2016_MPI_IO_H

#include <mpi.h>
#include <cstdio>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include "Tick.h"

using namespace std;

void read_data(MPI_File &in, const int rank, const int size, MPI_Status status, MPI_Offset &cycle_start,
               MPI_Offset &optimal_buffer_size, const int &overlap, char **chunk, char **real_start, char **real_end);

void parse_data(char *const *start, char *const *end, vector<Tick> *tick_data);

#endif //BDIF2016_MPI_IO_H
