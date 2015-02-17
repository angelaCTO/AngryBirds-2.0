/*
*  Sets up a serial communication between the Arduino Pro Mini and the 
*  Beaglebone Black. Listens for the Beaglebone Black to ask for the date 
*  and time from the RTC and sends back the corresponding data.
*
*  Written by Luke DeLuccia
*  Date: 07/31/14  
*/

#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"

#define rxPin 2   // RX pin for serial
#define txPin 3   // tx pin for serial
#define sclPin A5 // SCL pin for RTC
#define sdaPin A4 // SDA pin for RTC
 
char charIn; // the character being read from the Beaglebone Black 
String dateAndTimeToWrite; // the time to be written to the Beaglebone Black
DateTime now; // the current date and time
String hour, minute, second, month, day, year; // time and date strings

// software serial : RX = digital pin 2, TX = digital pin 3
SoftwareSerial serial(rxPin,txPin);

// Real-Time Clock
RTC_DS1307 RTC;

void setup() {
  
  Serial.begin(115200); // serial monitor baud rate
  
  // software serial port initialization
  serial.begin(115200); // baud rate of the mini's serial port
  serial.listen(); // listen on this port
  
  // RTC initialization
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  // global variable initialization
  dateAndTimeToWrite = "";
}

void loop() {
  
  now = RTC.now(); // grab the current date and time
  hour = "" + now.hour(); minute = "" + now.minute(); second = "" + now.second();
  month = "" + now.month(); day = "" + now.day(); year = "" + now.year();
  dateAndTimeToWrite = hour + ":" + minute + ":" + second + " " 
                       + month + " " + day + " " + year + "\n"; // **NEED \n
  
  // if the Beaglebone Black is talking...
  if (serial.available() > 0) { 
    Serial.println("Connection available!");
    Serial.print("Data from port one:");
    charIn = serial.read(); // read in the character
    Serial.println(charIn);
    // write the date and time
      for (int i = 0; i < dateAndTimeToWrite.length(); i++)
        serial.write(dateAndTimeToWrite.charAt(i));
    delay(5000); // delay a few seconds so we don't send the time more than once
  }
  else 
    Serial.println("No connection");
  delay(500); // check if the Beaglebone Black is talking every half second
}
