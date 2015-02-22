#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define LEFT_REV 2//left
#define RGHT_FWD 3//right
#define BRAKES   4//brake
#define UNK      5//??
#define RGHT_REV 6//right
#define LEFT_FWD 7//left

char text[1];
RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
unsigned long time;
unsigned long lastCommand;

void setup(){
  Serial.begin(57600);
  pinMode(LEFT_FWD,OUTPUT);
  pinMode(LEFT_REV,OUTPUT);
  pinMode(RGHT_FWD,OUTPUT);
  pinMode(RGHT_REV,OUTPUT);
  pinMode(BRAKES,OUTPUT);
  pinMode(UNK,OUTPUT);
  digitalWrite(LEFT_FWD,LOW);
  digitalWrite(LEFT_REV,LOW);
  digitalWrite(RGHT_FWD,LOW);
  digitalWrite(RGHT_REV,LOW);
  digitalWrite(BRAKES,LOW);
  digitalWrite(UNK,LOW);
  
  radio.begin(); 
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.setChannel(65);
  radio.startListening();
}

void loop(){
  int i;
  while(!radio.available()){}
  while(!radio.read(text,4)){}
  for(i=0;i<strlen(text);i++){
    Serial.print(text[i]);
  }
  Serial.print('!');
  switch (text[0]) {
    case 'w':
      digitalWrite(LEFT_FWD,HIGH);
      digitalWrite(LEFT_REV,LOW);
      digitalWrite(RGHT_FWD,HIGH);
      digitalWrite(RGHT_REV,LOW);
      Serial.println("FWD");
      break;
    case 's'://
      digitalWrite(LEFT_FWD,LOW);
      digitalWrite(LEFT_REV,HIGH);
      digitalWrite(RGHT_FWD,LOW);
      digitalWrite(RGHT_REV,HIGH);
      Serial.println("REV");
      break;
    case 'a'://
      digitalWrite(LEFT_FWD,LOW);
      digitalWrite(LEFT_REV,HIGH);
      digitalWrite(RGHT_FWD,HIGH);
      digitalWrite(RGHT_REV,LOW);
      Serial.println("LFT");
      break;
    case 'd'://
      digitalWrite(LEFT_FWD,HIGH);
      digitalWrite(LEFT_REV,LOW);
      digitalWrite(RGHT_FWD,LOW);
      digitalWrite(RGHT_REV,HIGH);
      Serial.println("RGT");
      break;
    /*case 'q':
      digitalWrite(BRAKES,HIGH);
      break;
    case 'e':
      digitalWrite(UNK,HIGH);*/
    default:
      digitalWrite(LEFT_FWD,LOW);
      digitalWrite(LEFT_REV,LOW);
      digitalWrite(RGHT_FWD,LOW);
      digitalWrite(RGHT_REV,LOW);
      digitalWrite(BRAKES,LOW);
      digitalWrite(UNK,LOW);
      Serial.println("Nothing");
      break;
  }
}
