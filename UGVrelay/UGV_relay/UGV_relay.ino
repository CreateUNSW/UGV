/*
UGV Relay Driver
CREATE UNSW
Written by: Austin Kong, Nathan Adler
First written:  2015 02 22
Last modified: 2015 02 22

Drives the UGV through an improvised motor driver using only relays.
Use of relays means there is no speed control.
PCB by William Hales 

Wired version, communicates over serial comms.
Shouldn't take too much to modify it so it can operate over Bluetooth that uses serial.

*/

#include <SoftwareSerial.h>

//Relay pin definitoins
#define LEFT_REV 2
#define RGHT_FWD 3
#define BRAKES   4
#define UNK      5 // Spare pin
#define RGHT_REV 6
#define LEFT_FWD 7

void setup() {
  Serial.begin(57600);
  
  //Setup pins
  pinMode(LEFT_FWD,OUTPUT);
  pinMode(LEFT_REV,OUTPUT);
  pinMode(RGHT_FWD,OUTPUT);
  pinMode(RGHT_REV,OUTPUT);
  pinMode(BRAKES,OUTPUT); // Brakes latch when pulled high
  pinMode(UNK,OUTPUT);
  digitalWrite(LEFT_FWD,LOW);
  digitalWrite(LEFT_REV,LOW);
  digitalWrite(RGHT_FWD,LOW);
  digitalWrite(RGHT_REV,LOW);
  digitalWrite(BRAKES,LOW);
  digitalWrite(UNK,LOW);
}

char in;
unsigned long lastCommand;

void loop() {
  if(Serial.available()) {
    while(Serial.available()){
      // Always flush out the buffer first
      in = Serial.read();
    }
    switch (in) {
      case 'w':
        digitalWrite(LEFT_FWD,HIGH);
        digitalWrite(LEFT_REV,LOW);
        digitalWrite(RGHT_FWD,HIGH);
        digitalWrite(RGHT_REV,LOW);
        break;
      case 's'://
        digitalWrite(LEFT_FWD,LOW);
        digitalWrite(LEFT_REV,HIGH);
        digitalWrite(RGHT_FWD,LOW);
        digitalWrite(RGHT_REV,HIGH);
        break;
      case 'a'://
        digitalWrite(LEFT_FWD,LOW);
        digitalWrite(LEFT_REV,HIGH);
        digitalWrite(RGHT_FWD,HIGH);
        digitalWrite(RGHT_REV,LOW);
        break;
      case 'd'://
        digitalWrite(LEFT_FWD,HIGH);
        digitalWrite(LEFT_REV,LOW);
        digitalWrite(RGHT_FWD,LOW);
        digitalWrite(RGHT_REV,HIGH);
        break;
      /*case 'q':
        digitalWrite(BRAKES,HIGH);
        break;
      case 'e':
        digitalWrite(UNK,HIGH);*/
      default:
        break;
    }
  } else if ((millis() - lastCommand) > 100) {
    // Resets driving state to stopped if no command is recieved
    lastCommand = millis();
    digitalWrite(LEFT_FWD,LOW);
    digitalWrite(LEFT_REV,LOW);
    digitalWrite(RGHT_FWD,LOW);
    digitalWrite(RGHT_REV,LOW);
    digitalWrite(BRAKES,LOW);
    digitalWrite(UNK,LOW);
  } 
}
