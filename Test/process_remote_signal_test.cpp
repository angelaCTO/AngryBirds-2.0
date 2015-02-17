#include <time.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "../BlackLib/BlackLib.h"

using namespace std;

void flash_led(int num_times);

int main() {
   // Change/Check GPIO Pin #
   BlackGPIO *ir_in = new BlackGPIO(GPIO_68, input);       // P8_10

   while (1) {
       if (ir_in->fail()) {
           cout << "ERROR" << endl;
       }
       cout << "GPIO pin value: " << ir_in->getValue() << endl;
       if (ir_in->isHigh()) {
           cout << "SIGNAL HEARD!" << endl;
           flashLed(5);
       }
    } 
}




/* 
 */
/* Flash the LED (P9_11 - GPIO 30)
 */
void flashLed(int numTimes) {
    // Set up the output pin for controlling the LED light
    BlackGPIO *ledOut = new BlackGPIO(GPIO_30, output);  // P9_11

    int i = 0;
    while (i < numTimes) {
        ledOut->setValue(high);
        sleep(1);
        ledOut->setValue(low);
        sleep(1);
        i++;
    }

