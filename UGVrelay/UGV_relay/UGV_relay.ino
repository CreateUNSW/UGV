//#include <SofwareSerial.h>

#define LEFT_REV 2//left
#define RGHT_FWD 3//right
#define BRAKES   4//brake
#define UNK      5//??
#define RGHT_REV 6//right
#define LEFT_FWD 7//left

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
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
}

char in;
unsigned long lastCommand;

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()) {
    while(Serial.available()){
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
    lastCommand = millis();
    digitalWrite(LEFT_FWD,LOW);
    digitalWrite(LEFT_REV,LOW);
    digitalWrite(RGHT_FWD,LOW);
    digitalWrite(RGHT_REV,LOW);
    digitalWrite(BRAKES,LOW);
    digitalWrite(UNK,LOW);
  } 
}
