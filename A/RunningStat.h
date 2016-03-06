//
// Created by alex on 2/28/16.
//

#ifndef BDIF2016_RUNNINGSTAT_H
#define BDIF2016_RUNNINGSTAT_H

#include <math.h>

class RunningStat {
public:
        RunningStat() : n(0),mean(0),M2(0),M3(0),M4(0) {}

        void Clear() {
            n = 0;
            mean=0;
            M2=0;
            M3=0;
            M4=0;
        }

        void Push(double x);

        int NumDataValues() const {
            return n;
        }

        double Mean() const {
            return (n > 0) ? mean : 0.0;
        }

        double Variance() const {
            return ((n > 1) ? M2 / (n - 1) : 0.0 );
        }

        double StandardDeviation() const {
            return sqrt( Variance() );
        }

        double Kurtosis() const {
            return (n*M4) / (M2*M2) - 3;
        }

        double Skewness() const {
            return sqrt(n / M2) * (M3 / M2);
        }

    private:
        int n;
        double mean,M2,M3,M4;
};


#endif //BDIF2016_RUNNINGSTAT_H
