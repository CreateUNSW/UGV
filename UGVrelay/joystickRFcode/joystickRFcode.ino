/*
UGV Joystick Remote
CREATE UNSW
Written by: Yunzhen Zhang, Austin Kong
First written:  2015 02 22
Last modified: 2015 02 22

Operates wirelessly with Nordic Semiconducor nRF24L01 2.4GHz over SPI.

For use with UGV Relay Driver (UGV_relay_RF.ino)
Use of relays means there is no speed control.
Hence analogue joystick position is broken into quadrants which equate to simple WSAD commands.

Uses a standard analogue joystick which uses two potentiometers on the two axis.

Should consider debouncing/smoothing if using raw analogue read values (no quadrants) in the future.

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

// Variables for RF module
RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

float x, y, r, theta;
char dir = ' ';

//SoftwareSerial bluetooth(10, 11); //RX, TX

void setup() {
  Serial.begin(57600);
  // Setup pins
  pinMode(VERTICAL, INPUT);
  pinMode(HORIZONTAL, INPUT);
  
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
  r = sqrt(x*x + y*y);
  theta = atan2(y,x);
  
  // Which quadrant is the joystick in?
  if (r > 10) { // Is the joystick outside the dead zone around home?
    if ((theta < FIRST_OCTANT)&&(theta > -FIRST_OCTANT)) {
      dir = 'd'; // Right
    } else if ((theta >= FIRST_OCTANT)&&(theta < THIRD_OCTANT)){     
      dir = 'w'; // Forward
    } else if ((theta >= THIRD_OCTANT)||(theta < -THIRD_OCTANT)) {
      dir = 'a'; // Left
    } else if ((theta >= -THIRD_OCTANT)&&(theta <= -FIRST_OCTANT)) {
      dir = 's'; // Reverse
    }
  } else {
    dir = ' '; // Joystick is in its home position
  }
  
  // Transmit character over RF
  // Need to stop and start listening becasue thats how we got it to work...
  // DISABLED SO IT STOPS FASTER Only transmit if joystick is not at home.
  //if (dir != ' ') {
    radio.stopListening();
    radio.write(&dir, 1);
    radio.startListening();
  //}
  
  //Debug printer
  Serial.print("X: ");
  Serial.print(x);
  Serial.print("  Y: ");
  Serial.print(y);
  Serial.print("  r: ");
  Serial.print(r);
  Serial.print("  theta: ");
  Serial.print(180* theta/(3.1415926));
  Serial.print("  direction: ");
  Serial.print(dir);
  Serial.println(""); 
  
  delay(100);
}
