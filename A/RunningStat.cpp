//
// Created by alex on 2/28/16.
//

#include "RunningStat.h"

//void RunningStat::Push(double x) {
//        n++;
//
//        // See Knuth TAOCP vol 2, 3rd edition, page 232
//        if (n == 1)
//        {
//            m_oldM = mean = x;
//            m_oldS = 0.0;
//        }
//        else
//        {
//            double delta = x - m_oldM;
//            mean = m_oldM + delta / n;
//            M2 = m_oldS + delta * (x - mean);
//
//            // set up for next iteration
//            m_oldM = mean;
//            m_oldS = M2;
//        }
//}


void RunningStat::Push(double x) {
    // from WikiPedia: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Higher-order_statistics
    long n1=n;
    ++n;
    double delta=x-mean;
    double delta_n=delta/n;
    double delta_n2=delta_n*delta_n;
    double term1=delta*delta_n*n1;
    mean+=delta_n;
    M4+=term1 * delta_n2 * (n*n - 3*n + 3) + 6 * delta_n2 * M2 - 4 * delta_n * M3;
    M3+=term1 * delta_n * (n - 2) - 3 * delta_n * M2;
    M2+=term1;
}