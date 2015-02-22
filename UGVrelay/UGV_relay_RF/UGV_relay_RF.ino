/*
UGV Relay Driver (Wireless via nRF24)
CREATE UNSW
Written by: Austin Kong, Yunzhen Zhang, Nathan Adler
First written:  2015 02 22
Last modified: 2015 02 22

Drives the UGV through an improvised motor driver using only relays.
Use of relays means there is no speed control.
PCB by William Hales 

Operates wirelessly with Nordic Semiconducor nRF24L01 2.4GHz over SPI.

*/

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

// Relay pin definitions
#define LEFT_REV 2
#define RGHT_FWD 3
#define BRAKES   4
#define UNK      5 // Spare pin
#define RGHT_REV 6
#define LEFT_FWD 7

// Variables for RF module
char text[1];
RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
unsigned long time;

unsigned long lastCommand;

void setup(){
  Serial.begin(57600);
  
  // Setup pins
  pinMode(LEFT_FWD,OUTPUT);
  pinMode(LEFT_REV,OUTPUT);
  pinMode(RGHT_FWD,OUTPUT);
  pinMode(RGHT_REV,OUTPUT);
  pinMode(BRAKES,OUTPUT); //Brakes latch when pulled high
  pinMode(UNK,OUTPUT);
  digitalWrite(LEFT_FWD,LOW);
  digitalWrite(LEFT_REV,LOW);
  digitalWrite(RGHT_FWD,LOW);
  digitalWrite(RGHT_REV,LOW);
  digitalWrite(BRAKES,LOW);
  digitalWrite(UNK,LOW);
  
  // Setup radio
  radio.begin(); 
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.setChannel(65);
  radio.startListening();
  
  lastCommand = millis();
}

void loop(){
  int i;
  if(radio.available()){
    while(!radio.read(text,4)){} // Does this flush out the buffer?
    // Debug printer
    for(i=0;i<strlen(text);i++){
      Serial.print(text[i]);
    }
    Serial.print('!'); // Just a marker for end of message.
    switch (text[0]) {
      case 'w':
        lastCommand = millis();
        go_forward();
        Serial.println("FWD");
        break;
      case 's'://
        lastCommand = millis();
        go_backward();
        Serial.println("REV");
        break;
      case 'a'://
        lastCommand = millis();
        go_left();
        Serial.println("LFT");
        break;
      case 'd'://
        lastCommand = millis();
        go_right();
        Serial.println("RGT");
        break;
      /*case 'q':
        digitalWrite(BRAKES,HIGH);
        break;
      case 'e':
        digitalWrite(UNK,HIGH);*/
      default: 
        // Joystick defaults to sending spaces when homed.
        stop_driving();
        Serial.println("Nothing");
        break;
    }
  } else if (millis()-lastCommand>400) {
    // Resets driving state to stopped if no command is recieved for a while.
    stop_driving();
    lastCommand = millis();
  }   
}

void go_forward(){
  digitalWrite(LEFT_FWD,HIGH);
  digitalWrite(LEFT_REV,LOW);
  digitalWrite(RGHT_FWD,HIGH);
  digitalWrite(RGHT_REV,LOW);
}

void go_backward(){
  digitalWrite(LEFT_FWD,LOW);
  digitalWrite(LEFT_REV,HIGH);
  digitalWrite(RGHT_FWD,LOW);
  digitalWrite(RGHT_REV,HIGH);
}

void go_left(){
  digitalWrite(LEFT_FWD,LOW);
  digitalWrite(LEFT_REV,HIGH);
  digitalWrite(RGHT_FWD,HIGH);
  digitalWrite(RGHT_REV,LOW);
}

void go_right(){
  digitalWrite(LEFT_FWD,HIGH);
  digitalWrite(LEFT_REV,LOW);
  digitalWrite(RGHT_FWD,LOW);
  digitalWrite(RGHT_REV,HIGH);
}

void stop_driving(){
  digitalWrite(LEFT_FWD,LOW);
  digitalWrite(LEFT_REV,LOW);
  digitalWrite(RGHT_FWD,LOW);
  digitalWrite(RGHT_REV,LOW);
  digitalWrite(BRAKES,LOW);
  digitalWrite(UNK,LOW);
}
