
/******************************
* PIN DECLARATIONS
******************************/

int IRledPin =  10;   // LED connected to digital pin 13
int BttnPin = 11;      // Button connected to pin 2

void setup()   {                
  pinMode(IRledPin, OUTPUT);  
  pinMode(BttnPin, INPUT);
  Serial.begin(9600);
}
 
void loop(){  
  if (digitalRead(BttnPin)){
    Serial.println("Sending IR signal");
    SendIRCode();
  }

}
 
// This procedure sends a 38KHz pulse to the IRledPin 
// for a certain # of microseconds. We'll use this whenever we need to send codes
void pulseIR(long microsecs) {
  // we'll count down from the number of microseconds we are told to wait
 
  cli();  // this turns off any background interrupts
 
  while (microsecs > 0) {
    // 38 kHz is about 13 microseconds high and 13 microseconds low
   digitalWrite(IRledPin, HIGH);  // this takes about 3 microseconds to happen
   delayMicroseconds(10);         // hang out for 10 microseconds, you can also change this to 9 if its not working
   digitalWrite(IRledPin, LOW);   // this also takes about 3 microseconds
   delayMicroseconds(10);         // hang out for 10 microseconds, you can also change this to 9 if its not working
 
   // so 26 microseconds altogether
   microsecs -= 26;
  }
 
  sei();  // this turns them back on
}

void SendIRCode(){
  pulseIR(1500);
  delay(20);
  pulseIR(1000);
  delayMicroseconds(2000);
  pulseIR(1000);
  delayMicroseconds(3000);
  pulseIR(2000);
  delayMicroseconds(6000);
  pulseIR(2000);  
  
  delay(65); // wait 65 milliseconds before sending it again  
  
  pulseIR(1500);
  delay(20);
  pulseIR(1000);
  delayMicroseconds(2000);
  pulseIR(1000);
  delayMicroseconds(3000);
  pulseIR(2000);
  delayMicroseconds(6000);
  pulseIR(2000);
}

void SendNikonCode() {
  // This is the code for my particular Nikon, for others use the tutorial
  // to 'grab' the proper code from the remote
 
  pulseIR(2080);
  delay(27);
  pulseIR(440);
  delayMicroseconds(1500);
  pulseIR(460);
  delayMicroseconds(3440);
  pulseIR(480);
 
 
  delay(65); // wait 65 milliseconds before sending it again
 
  pulseIR(2000);
  delay(27);
  pulseIR(440);
  delayMicroseconds(1500);
  pulseIR(460);
  delayMicroseconds(3440);
  pulseIR(480);
}
