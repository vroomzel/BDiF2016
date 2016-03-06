//
// Created by alex on 2/28/16.
//

#ifndef BDIF2016_LOG_H
#define BDIF2016_LOG_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <math.h>

#include "Parameters.h"

using namespace std;

enum LogTypeEnum {
	CALLED,		// Function call.
	DIAGNOSTIC,	// Diagnostic output.
	EVOLVE,		// Evolution of State and Exposures along simulation paths.
	EXIT,		// Error has occured and program will exit.
	INFO,		// Informational message.
	NAME,		// Simulation name.
	RESULT		// Results.
};
// string representations of enums
static std::string pLogTypeEnum[] = { "CALLED", "DIAGNOSTIC", "EVOLVE","EXIT","INFO","NAME","RESULT" };

enum ModuleNameEnum {
    MAIN,MPIIO,TICK,LOG_MODULE,HELPER_FUNCTIONS
};
static string pModuleNameEnum[] = {"MAIN","MPI_IO","TICK","LOG","HELPER_FUNCTIONS"};

void LOG(const Parameters &params, LogTypeEnum t, ModuleNameEnum m, string message);
void LOG(const Parameters &params, LogTypeEnum t, ModuleNameEnum m, string message1, string message2);
void LOG(const Parameters &params, LogTypeEnum t, ModuleNameEnum m, string message1, string message2, string message3);
void LOG(const Parameters &params, LogTypeEnum t, ModuleNameEnum m, string message1, string message2, string message3, string message4);

static clock_t start;
string ElapsedTime();

string Timestamp();

#endif //BDIF2016_LOG_H
