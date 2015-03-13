/*
UGV Joystick Remote
CREATE UNSW
Written by: Nathan, using code by Austin and Winnie
First written:  2015 03 13
Last modified: 2015 03 13

Operates wirelessly with Nordic Semiconducor nRF24L01 2.4GHz over SPI.

For use with UGV Relay Driver (UGV_relay_RF.ino)
Use of relays means there is no speed control.
Hence analogue joystick position is encoded into 32 bit packets and send over RF.

Uses a standard analogue joystick which uses two potentiometers on the two axis.

Should consider debouncing/smoothing if using raw analogue read values.

*/

#include <SoftwareSerial.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

// Joystick home position values
#define CENTRE_X 521
#define CENTRE_Y 516
// Joystick pinouts
#define VERTICAL A0
#define HORIZONTAL A1
// Angle contants to mark each quadrant (in radians).
#define FIRST_OCTANT 0.7854
#define THIRD_OCTANT 2.3562

typedef struct {
  float rawMag;
  float rawAngle;
} rf_data_t;

// Variables for RF module
RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
rf_data_t rfData;

float x, y, r, theta;

//SoftwareSerial bluetooth(10, 11); //RX, TX

void setup() {
  Serial.begin(57600);
  // Setup pins
  //pinMode(VERTICAL, INPUT);
  //pinMode(HORIZONTAL, INPUT);
  
  // Setup radio
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.setChannel(65);
  radio.startListening();
}

void loop() {
  // Read inputs
  x = analogRead(HORIZONTAL) - CENTRE_X;
  y = analogRead(VERTICAL) - CENTRE_Y;
  // Convert to polar coordinates
  r = sqrt(x*x + y*y)/512;
 
  theta = atan2(y,x);
  if(abs(x)>abs(y)){
    r*=abs(cos(theta));
  } else {
    r*=abs(sin(theta));
  }
  rfData.rawMag = min(r,1);
  rfData.rawAngle = theta;
  // Which quadrant is the joystick in?
 
  
  // Transmit character over RF
  // Need to stop and start listening becasue thats how we got it to work...
  // DISABLED SO IT STOPS FASTER Only transmit if joystick is not at home.
  if (r>0.05) {
    radio.stopListening();
    radio.write(&rfData, 8);
    radio.startListening();
  }
  
  //Debug printer
  Serial.print("X: ");
  Serial.print(x);
  Serial.print("  Y: ");
  Serial.print(y);
  Serial.print("  r: ");
  Serial.print(r);
  Serial.print("  theta: ");
  Serial.print(180* theta/(3.1415926));
  Serial.println(""); 
  
  delay(100);
}
