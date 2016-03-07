//
// Created by alex on 2/17/16.
//

#ifndef BDIF2016_TICK_H
#define BDIF2016_TICK_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <time.h>
#include <vector>
#include <limits>
#include <fstream>
#include <cstring>

// my includes
#include "enums.h"
#include "helper_functions.h"

using namespace std;

class Tick {
public:
    // attributes
    TickStatusEnum status;
//    double timeStamp;
    int date,hour,minute;
    float second;
    float price;
    long volume;
//    vector<string> datas; // might be useful to keep in order to output bad ticks later
//    char* datac;
    char const *beg, *end; //pointers to begginning and ending of the string in the buffer

//constructors
    Tick();
    Tick(const char *beg, const char *end);
//    Tick(vector<string> datas);
//    Tick(char* datac);

    //methods
    string toString() const;

    //operator overloads
    bool operator < (const Tick&) const;
    bool operator == (const Tick&) const;

};


#endif //BDIF2016_TICK_H
