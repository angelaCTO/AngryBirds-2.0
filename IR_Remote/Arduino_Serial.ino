/*
 *  Sets up a serial communication between the Arduino Pro Mini and the 
 *  Beaglebone Black. Listens for a signal sent by the IR Remote to 
 *  indicate time to sleep/kill video recording script
 *
 *  Written by Luke DeLuccia
 *  Date: 07/31/14  
 */

#include <SoftwareSerial.h>
#include <Wire.h>

#define rxPin 2   // RX pin for serial
#define txPin 3   // tx pin for serial
#define sclPin A5 // SCL pin for RTC
#define sdaPin A4 // SDA pin for RTC
 
char charIn;      // The char being read from the Beaglebone Black 

// Software serial : RX = digital pin 2, TX = digital pin 3
SoftwareSerial serial(rxPin,txPin);


void setup() {
  // Set up the serial monitor baud rate
  Serial.begin(115200);                 
  
  // Software serial port initialization - set the baud rate 
  // of the mini's serial port
  serial.begin(115200);       
  serial.listen();                        
 
  // Initialization
  Wire.begin();             
 
  // Global variable initialization
  ir_signal = "";
}

void loop() {
  // Try to establish a connection with BBB
  if (serial.available() > 0) { 
    Serial.println("Connection available!");
    Serial.print("Data from port one:");
    charIn = serial.read();               // Read in the character
    Serial.println(charIn);

    // ir_signal = "sleep";
    
    // Write the (IR) signal flag to the BBB
    for (int i = 0; i < ir_signal.length(); i++) {
        serial.write(ir_signal.charAt(i));
    }
    delay(5000); 
  } else {
    Serial.println("No connection");
  }
  delay(500); // check if the Beaglebone Black is talking every half second
}
