#ifndef SENSOR_SIGNAL_H
#define SENSOR_SIGNAL_H

#define SAMPLE_SIZE 0 //!! Need to change

#include <deque>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include "../BlackLib/BlackLib.h"

using namespace std;

class SensorSignal
{
//    private:
//        deque<int> sensor_signal;

    public:
        SensorSignal();
        void build_signal_deque(deque<int> &signal_deque, 
                                const int window_size);
        float compute_normal_signal(deque<int> &signal_deque, 
                                    deque<int> &averaged_deque, 
                                    const int window_size);
        float compute_average_signal(deque<int> &signal_deque, 
                                     const int window_size);
         
};

#endif // EOF


