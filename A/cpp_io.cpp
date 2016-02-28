//
// Created by alex on 2/16/16.
//

#include "cpp_io.h"
const int MAX_LINES = 100000;

void read_file(string fname, const int proc_rank, const int worldsize){
    ifstream datafile(fname);
    // figure out file size
    streampos begin,end;
    begin=datafile.tellg();
    datafile.seekg(0,ios::end);
    end=datafile.tellg();
    long long filesize=end-begin;
    // split the file between processes
    long long localSize = filesize / worldsize;
    long long localBegin,localEnd;
    if (proc_rank == 0)
        localBegin=begin;
    else
        localBegin= localSize * proc_rank + 1;
    if (proc_rank == worldsize - 1)
        localEnd=end;
    else
        localEnd= (proc_rank + 1) * localSize;
    /*
     * Move to starting position
     * Read the rest of the line and discard it - it'll be read by previous file
     * Read lines until your current position is greater than localEnd - you'll end up reading a bit behind the localEnd
     * unless you dealing with the last chunk
     */
    datafile.seekg(localBegin);
    string line;
    // read remainder of first line and discard it
    if (proc_rank != 0) getline(datafile, line);
    while (datafile.tellg()<localEnd){
        int linesRead=0;
        while ((datafile.tellg()<localEnd) & (linesRead<=MAX_LINES)){
            getline(datafile,line);
            ++linesRead;
            // split the line into timestamp, price and volume
            int cutAt;
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
            // convert string data into ticks
            Tick newTick(datas);
            cout<<newTick.toString()<<endl;
        }
    }
    datafile.close();
}
