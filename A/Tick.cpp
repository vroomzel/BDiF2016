//
// Created by alex on 2/17/16.
//

#include <sstream>
#include "Tick.h"

const short int OUTPUT_PRECISION=17;

//constructors
Tick::Tick(){};

string Tick::toString() {
    stringstream out;
    out.precision(OUTPUT_PRECISION);
    out << "Date: " << this->date << " H: " << this->hour << " M: " << this->minute<<" S: "<<this->second
    <<" Px: "<<this->price<<" Vol: "<<this->volume<< " Status: " << pTickStatusEnum[this->status];
    return out.str();
}

Tick::Tick(const char *beg, const char *end) {
    this->beg=beg;
    this->end=end;
    string line(beg,end);
    // parse timestamp
    int cutAt = line.find_first_of(",");
    string tmp = line.substr(0,cutAt);
    this->date=atoi(tmp.substr(0,8).c_str());
    this->hour=atoi(tmp.substr(9,2).c_str());
    this->minute=atoi(tmp.substr(12,2).c_str());
    this->second=atof(tmp.substr(15).c_str());
    // parse price
    line = line.substr(cutAt+1);
    cutAt = line.find_first_of(",");
    this->price=atof(line.substr(0,cutAt).c_str());
    line = line.substr(cutAt+1);
    this->volume=atol(line.c_str());

    if ((price<5e-6) || (volume<5e-6) || hour>16 || hour<9)
        this->status = TickStatusEnum::BAD;
    else
        this->status=TickStatusEnum::GOOD;
}



