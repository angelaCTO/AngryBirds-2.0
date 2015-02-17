/*
 * Defines functions in order for the Beaglebone Black to receive 
 * the sleep flag from the Arduino Pro Mini through a serial 
 * connection.
 *
 * Written by Luke DeLuccia
 * Date: 7/31/14
 */

#include <stdio.h>
#include <string>
#include "serialib.h"
#include "../BlackLib/BlackLib.h"

// ttyO2 for linux = UART2 for Beaglebone Black
#define  DEVICE_PORT  "/dev/ttyO2" 
#define  BAUD_RATE    115200
                                  
// Object of the serialib class used for the return values                 
Serialib LS;          
int Ret;                         
char Buffer[128];


/* Listens for a flag to be sent from the Arduino to BBB to 
 * indicate time to sleep
 */
bool listenForSleepSignal() {
    openSerialPort();
    // The key that will be used to compare against incoming signals 
    // to determine if it is time to sleep the system
    string sleepKey = "sleep";
    while (1) {
        // Listen for a signal to be recieved from the Arduino
        Ret = LS.ReadString(Buffer, '\n', 128, 5000);
        // The final character of the string must be aline feed ('\n')
        if (Ret > 0) {
            // Check if recieved string matches our sleep key. 
            // If a match is found, break and return true;
            if (Buffer.compare(sleepKey) == 0) {
                break;
            }
        }
    }
    closeSerialPort();
    return true;
}






/* If a sleep signal has been recieved, sleep the main (video capturing)
 * proces for 30 minutes
 */
void processSignal() {
   // Flash LED to indicate that signal has been detected 
   flashLed(ledOut, 5)

   // DO SOMETHING HERE

   // Flash LED again to indicate that system is now asleep 
   flashLed(ledOut, 5)
}


/* Flash the LED (P8_10 - GPIO 68)
 */
void flashLed(BlackGPIO *ledOut, int numTimes) {
    // Set up the output pin for controlling the LED light
    BlackGPIO *ledOut = new BlackGPIO(GPIO_68, output);  // P8_10

    int i = 0;
    while (i < num_times) {
        led_out->setValue(high);
        sleep(1);
        led_out->setValue(low);
        sleep(1);
        i++;
    }
}


/*
 * Opens the serial port between the Beaglebone Black and the Arduino
 * Pro Mini at the given BAUD_RATE
 */
void openSerialPort(){
    // Open serial link at 115200 bauds
    Ret = LS.Open(DEVICE_PORT,BAUD_RATE);  
    // Check if an error has occured
    if (Ret != 1) {                            
        printf ("Error while opening port. Permission problem ?\n");    
        return;                                                   
    }
    printf ("Serial port opened successfully !\n");
}


/*
 * Closes the serial port between the Beaglebone Black and the Arduino 
 * Pro Mini
 */
void closeSerialPort(){
    LS.Close();
}
