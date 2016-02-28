//
// Created by alex on 2/16/16.
//

#ifndef BDIF2016_MPI_IO_H
#define BDIF2016_MPI_IO_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <time.h>
#include <vector>
#include <limits>
#include <fstream>
#include <cstring>
#include "mpi.h"

// my includes
#include "Tick.h"

using namespace std;

void read_file(string fname, const int proc_rank, const int worldsize);







#endif //BDIF2016_MPI_IO_H
