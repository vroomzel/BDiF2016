//
// Created by alex on 2/17/16.
//

#ifndef BDIF2016_HELPER_FUNCTIONS_H
#define BDIF2016_HELPER_FUNCTIONS_H

#include <iostream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <time.h>
#include <typeinfo>

using namespace std;

double getTimestampFromDateTime(string strDateTime);

vector<string> parse_string (const string str,const char delimiter);

#endif //BDIF2016_HELPER_FUNCTIONS_H
