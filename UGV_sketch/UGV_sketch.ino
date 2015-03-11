/**
* UGV driving framework to replace properietary motor controlling system
* Written by: Nathan Adler
* Last modified: 21/2/2015
**/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define LEFT_MOTOR_DIR_PIN  2
#define LEFT_MOTOR_PWM_PIN  3

#define RIGHT_MOTOR_DIR_PIN  4
#define RIGHT_MOTOR_PWM_PIN  5

#define LEFT_BRAKE_PIN  6
#define RIGHT_BRAKE_PIN  7

typedef enum {SLOWEST=51,
              SLOW=102,
              MEDIUM=153,
              FAST=204,
              FASTEST=255} speed_scale_t;
              
typedef enum {FORWARD,REVERSE} direction_t;
typedef enum {LEFT,RIGHT} motor_type_t;

typedef enum {SUCCESS, DISCONNECT, FAULT, UNKNOWN} error_t;

typedef struct {
  float angle; //0-2*PI radians
  float magnitude;//0-1.0
} joystick_t;

typedef struct {
  float speed;//0-1.0
  direction_t direction;//forward or reverse
} motor_state_t;

typedef struct {
  motor_state_t leftMotor;
  motor_state_t rightMotor;
} ugv_state_t;

typedef struct {
  uint16_t rawMag;
  uint16_t rawAngle;
} rf_data_t;

static speed_scale_t currentSpeed = SLOWEST;
static joystick_t currentJoystick;
static ugv_state_t currentUGV;
static ugv_state_t desiredUGV;
static bool joystickReleased = true;
static bool brakeState = true;

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

/**Function declarations**/
void error_check(error_t code);

void desired_to_current_ugv(ugv_state_t *from,ugv_state_t *to);
void move_ugv(ugv_state_t ugv);

error_t init_motors();
error_t init_brakes();
error_t init_joystick();
void motor_out(motor_state_t motor, motor_type_t type);
uint8_t convert_speed(float speed);
void motors_off();
void change_speed(speed_scale_t newSpeed);
void brakes_engage();
void brakes_disengage();
void joystick_to_ugv(joystick_t joystick, ugv_state_t *ugv);
void get_joystick(joystick_t *joystick);

/**Error handler**/
void error_check(error_t code){
  if(code!=SUCCESS){
    // stop program from continuing
    motors_off();
    brakes_engage();
    while(1){}
  }
}
  
void setup(){
  /**Initialize hardware**/
  error_t errorCode;
  errorCode = init_motors();
  error_check(errorCode);
  errorCode = init_brakes();
  error_check(errorCode);
  errorCode = init_joystick();
  error_check(errorCode);
  
  /**Initialize desired state**/
  desiredUGV.leftMotor.speed = 0;
  desiredUGV.leftMotor.direction = FORWARD;
  desiredUGV.rightMotor.speed = 0;
  desiredUGV.rightMotor.direction = FORWARD;
  
}

void loop(){
  unsigned long joystickReleaseTime;
  while(1){
    get_joystick(&currentJoystick);
    if(currentJoystick.magnitude==0){
      motors_off();
      if(joystickReleased==false){
        joystickReleaseTime = millis();
        joystickReleased = true;
      }
      if(brakeState==false&&millis()-joystickReleaseTime>400){
        brakes_engage();
      }      
    } else {
      joystickReleased = false;  
      joystick_to_ugv(currentJoystick,&desiredUGV);
      desired_to_current_ugv(&desiredUGV,&currentUGV);
      move_ugv(currentUGV);
    }
  }
}

void move_ugv(ugv_state_t ugv){
  // Use "currentState" for this function
  if(brakeState==true){
    brakes_disengage();
  }
  motor_out(ugv.leftMotor,LEFT);
  motor_out(ugv.rightMotor,RIGHT);
}

void desired_to_current_ugv(ugv_state_t *from,ugv_state_t *to){
  // at the moment simply copying. May need to implement gradual changes
  memcpy(to,from,sizeof(ugv_state_t));
}

/**Initialization code**/

error_t init_motors(){
  pinMode(LEFT_MOTOR_DIR_PIN,OUTPUT);
  pinMode(LEFT_MOTOR_PWM_PIN,OUTPUT);
  pinMode(RIGHT_MOTOR_DIR_PIN,OUTPUT);
  pinMode(RIGHT_MOTOR_PWM_PIN,OUTPUT);
  //insert motor test checks here
  /**Initialize current motor state**/
  currentUGV.leftMotor.direction = FORWARD;
  currentUGV.rightMotor.direction = FORWARD;
  currentUGV.leftMotor.speed = 0;
  currentUGV.rightMotor.speed = 0;
  motors_off();
  return SUCCESS;
}

error_t init_brakes(){
  pinMode(LEFT_BRAKE_PIN,OUTPUT);
  pinMode(RIGHT_BRAKE_PIN,OUTPUT);
  //insert brake test checks here
  brakes_engage();
  return SUCCESS;
}

error_t init_joystick(){
  //insert joystick init code here
  currentJoystick.angle = 0;
  currentJoystick.magnitude = 0;
  // Setup radio
  radio.begin(); 
  //radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.setChannel(65);
  radio.startListening();
  //insert joystick test checks here
  return SUCCESS;
}

/**Motor driving code**/

void motor_out(motor_state_t motor, motor_type_t type){
  uint8_t dirPin,pwmPin;
  uint8_t speedVal;
  if(type==LEFT){
    dirPin = LEFT_MOTOR_DIR_PIN;
    pwmPin = LEFT_MOTOR_PWM_PIN;
  } else if(type==RIGHT){
    dirPin = RIGHT_MOTOR_DIR_PIN;
    pwmPin = RIGHT_MOTOR_PWM_PIN;
  } else {
    return;
  }
  speedVal = convert_speed(motor.speed);
  if(motor.direction==FORWARD){
    digitalWrite(dirPin,HIGH);
  } else if(motor.direction==REVERSE){
    digitalWrite(dirPin,LOW);
  }
  analogWrite(pwmPin,speedVal);
}

uint8_t convert_speed(float speed){
  float speedVal = speed*(float)currentSpeed;
  return (uint8_t)speedVal;
}

void motors_off(){
  digitalWrite(LEFT_MOTOR_PWM_PIN,LOW);
  digitalWrite(RIGHT_MOTOR_PWM_PIN,LOW);
}

void change_speed(speed_scale_t newSpeed){
  currentSpeed = newSpeed;
}

/**Brake operation code**/

void brakes_engage(){
  digitalWrite(LEFT_BRAKE_PIN,LOW);
  digitalWrite(RIGHT_BRAKE_PIN,LOW);
  brakeState = true;
}

void brakes_disengage(){
  digitalWrite(LEFT_BRAKE_PIN,HIGH);
  digitalWrite(RIGHT_BRAKE_PIN,HIGH);
  brakeState = false;
  delay(200);
}

/**Joystick code**/

void joystick_to_ugv(joystick_t joystick, ugv_state_t *ugv){
  if(sin(joystick.angle)>=0){
    ugv->leftMotor.direction = FORWARD;
    ugv->rightMotor.direction = FORWARD;
  } else {
    ugv->leftMotor.direction = REVERSE;
    ugv->rightMotor.direction = REVERSE;
  }
  if(cos(joystick.angle)>0){
    ugv->leftMotor.speed = 1;
    ugv->rightMotor.speed = cos(joystick.angle);
  } else if(cos(joystick.angle<0)){
    ugv->rightMotor.speed = 1;
    ugv->leftMotor.speed = abs(cos(joystick.angle));
  } else {
    ugv->leftMotor.speed = 1;
    ugv->rightMotor.speed = 1;
  }
}

void get_joystick(joystick_t *joystick){
  //insert joystick reading code here
  //note: must read magntitude scale 0-1 and
  //angle in radians where 0 is directly to the right,
  //going anticlockwise (conventional)
  if(radio.available()){
    rf_data_t rfData;
    while(!radio.read(&rfData,4)){}
    joystick->magnitude = (float)rfData.rawMag/65535;
    joystick->angle = ((float)rfData.rawAngle/65535)*2*PI;
  }
}
