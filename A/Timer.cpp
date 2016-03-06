//
// Created by alex on 2/28/16.
//

#include <stddef.h>
#include "Timer.h"

Timer::Timer(): d_time(0) {};

void Timer::start() {
    gettimeofday(&tv_start,NULL);
}

void Timer::end() {
    gettimeofday(&tv_end,NULL);
    d_time+=double(tv_end.tv_sec-tv_start.tv_sec)+double(tv_end.tv_usec-tv_start.tv_usec)/1e6;
}
