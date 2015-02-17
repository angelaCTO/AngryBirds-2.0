/* GPIO Output and LED Test 
 */

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "../BlackLib/BlackLib.h"

using namespace std;


void flash_led(BlackGPIO *led_out, int num_times);


int main() {
   BlackGPIO *led_out = new BlackGPIO(GPIO_30, output);  // P8_10
   cout << "LED_OUT VALUE: " << led_out->getValue() << endl;
   flash_led(led_out, 10);
}


void flash_led(BlackGPIO *led_out, int num_times) {
    cout << "IN FLASH LED" << endl;
    int i = 0;
    while (i < num_times) {
        cout << "i: " << i << endl;
   cout << "LED_OUT VALUE_start: " << led_out->getValue() << endl;
        led_out->setValue(high);
   cout << "LED_OUT VALUE_set high: " << led_out->getValue() << endl;
        sleep(1);     
        led_out->setValue(low);
   cout << "LED_OUT VALUE_set low: " << led_out->getValue() << endl;
        sleep(1);
        i++;
    }
}
