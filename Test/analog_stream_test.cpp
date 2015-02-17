#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "../BlackLib/BlackLib.h"

using namespace std;

int main() 
{
 				   // Ground = 34
    BlackADC *adc = new BlackADC(AIN4); // Pin 33 
    float analog; 
    float converted;
    int signal_count = 0;
   
    while (1) 
    {
        if (adc->fail()) 
        {
            cerr << "Error" << endl;
        }

        analog = adc->getNumericValue();
        converted = adc->getParsedValue(dap3);      

//        cout << "analog_input: " << analog << endl;
//        cout << "converted voltage: " << converted << endl; 
//        cout << "\n" << endl;

        if (analog > 1000 ){
              cout << "analog: " << analog << endl;
              cout << "\nSIGNAL DETECTED " << signal_count << endl;
              signal_count++;
              sleep(1);
        }
    }
}



