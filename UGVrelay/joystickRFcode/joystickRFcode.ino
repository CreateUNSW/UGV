#include <SoftwareSerial.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define CENTRE_X 521
#define CENTRE_Y 516
#define VERTICAL A0
#define HORIZONTAL A1
#define FIRST_OCTANT 0.7854
#define THIRD_OCTANT 2.3562

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

float x, y, r, theta;
char dir = ' ';

SoftwareSerial bluetooth(10, 11); //RX, TX

void setup() {
  pinMode(VERTICAL, INPUT);
  pinMode(HORIZONTAL, INPUT);
  //Serial.begin(115200);
  Serial.begin(57600);
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.setChannel(65);
  radio.startListening();
}

void loop() {
 x = analogRead(HORIZONTAL) - CENTRE_X;
 y = analogRead(VERTICAL) - CENTRE_Y;
 r = sqrt(x*x + y*y);
 theta = atan2(y,x);
 if (r > 10) {
   if ((theta < FIRST_OCTANT)&&(theta > -FIRST_OCTANT)) {
    dir = 'd'; //right
   } else if ((theta >= FIRST_OCTANT)&&(theta < THIRD_OCTANT)){     
    dir = 'w'; //forward
   } else if ((theta >= THIRD_OCTANT)||(theta < -THIRD_OCTANT)) {
    dir = 'a'; //left
   } else if ((theta >= -THIRD_OCTANT)&&(theta <= -FIRST_OCTANT)) {
    dir = 's'; //reverse
   }
 } else {
   dir = ' ';
 }
 radio.stopListening();
 radio.write(&dir, 1);
 radio.startListening();
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
