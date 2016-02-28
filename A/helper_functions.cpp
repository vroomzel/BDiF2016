//
// Created by alex on 2/17/16.
//

#include "helper_functions.h"

/*
 * from string "YYYYmmdd:hh:mm:ss.msmsmsmsms" to timestamp double
 */
double getTimestampFromDateTime(string strDateTime) {
    //determine timestamp
    string year = strDateTime.substr(0,4);
    string month = strDateTime.substr(4,2);
    string day = strDateTime.substr(6,2);
    string hour = strDateTime.substr(9,2);
    string minute = strDateTime.substr(12,2);
    string second = strDateTime.substr(15,2);
    string ms = strDateTime.substr(18);

    //timestamp
    struct tm dateTime;
    dateTime.tm_year = atoi(year.c_str()) - 1900;
    dateTime.tm_mon = atoi(month.c_str()) - 1;
    dateTime.tm_mday = atoi(day.c_str());
    dateTime.tm_hour = atoi(hour.c_str());
    dateTime.tm_min = atoi(minute.c_str());
    dateTime.tm_sec = atoi(second.c_str());
    dateTime.tm_isdst = -1; //tells mktime() to determine whether daylight saving time is in effect
    time_t datetime = mktime(&dateTime);

    //timestamp
    double timeStamp = (double)datetime;
    return timeStamp+atof(ms.c_str())/1000000;
}

vector<string> parse_string (const string str,const char delimiter){
    int cutAt;
    string line = str;
    vector<string> datas;
    while( (cutAt = line.find_first_of(",")) != line.npos )
    {
        if(cutAt > 0)
        {
            datas.push_back(line.substr(0, cutAt));
        }
        line = line.substr(cutAt+1);
    }
    if(line.length() > 0) {
        datas.push_back(line);
    }
    return datas;
}
