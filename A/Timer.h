//
// Created by alex on 2/28/16.
//

#ifndef BDIF2016_TIMER_H
#define BDIF2016_TIMER_H

#include <sys/time.h>

class Timer {
public:
    // members
    timeval tv_start;
    timeval tv_end;
    double d_time;

    //basic constructor
    Timer();

    // methods

    void start();
    void end();
};

#endif //BDIF2016_TIMER_H
