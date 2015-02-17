#include <time.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "../BlackLib/BlackLib.h"

void flashLed(int numTimes);

int main() {
   flashLed(10);
}

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
}
