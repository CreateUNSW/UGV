/**
 * UGV driving framework to replace properietary motor controlling system
 * Written by: Nathan Adler
 * First written: 21/2/2015
 * Last modified: 13/3/2015 by Austin and Sam. Fixed polar coordinates decoding.  
 *
 * Remotely driven with nRF24L01.
 * For use with UGV_new_joystick.ino
 **/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <SPI.h>
#include <I2Cdev.h>
#include <Wire.h>1
#include <AD7746.h>

#define LEFT_MOTOR_DIR_PIN  4
#define LEFT_MOTOR_PWM_PIN  5

#define RIGHT_MOTOR_DIR_PIN  2
#define RIGHT_MOTOR_PWM_PIN  3

#define LEFT_BRAKE_PIN  6
#define RIGHT_BRAKE_PIN  7

typedef enum {
  SLOWEST=51,
  SLOW=102,
  MEDIUM=153,
  FAST=204,
  FASTEST=255
} speed_scale_t;

typedef enum {FORWARD,REVERSE} direction_t;
typedef enum {LEFT,RIGHT} motor_type_t;
typedef enum {SUCCESS, DISCONNECT, FAULT, UNKNOWN} error_t;

typedef struct {
  float angle; //-PI to +PI radians
  float magnitude; // 0 to 1.0
} joystick_t;

typedef struct {
  float speed; //0-1.0
  direction_t direction; //FORWARD or REVERSE
} motor_state_t;

typedef struct {
  motor_state_t leftMotor;
  motor_state_t rightMotor;
} ugv_state_t;

typedef struct {
  float rawMag;
  float rawAngle;
} rf_data_t;

static speed_scale_t currentSpeed = FASTEST; // Hardcoded to be fastest for now
static joystick_t currentJoystick;           // Where the bot is currently pointed
static ugv_state_t currentUGV;
static ugv_state_t desiredUGV;
static bool joystickReleased = true;
static bool brakeState = true;
static unsigned long lastJoystickCommand;

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
  Serial.begin(9600);
  delay(2000);
}

unsigned long joystickReleaseTime;

void loop(){
  /* Loop must:
               recieve,
               do,
               send  */
  
  // First the recieve
  
  while (Serial.peek() == -1){
    // wait for an instruction
    delay(1);
    if(millis()-lastJoystickCommand>40000){ // TODO MAKE THIS MORE SIMPLE
      currentJoystick.angle = 0;
      currentJoystick.magnitude = 0;
      joystick_to_ugv(currentJoystick,&desiredUGV);
      desired_to_current_ugv(&desiredUGV,&currentUGV);
      move_ugv(currentUGV);
      brakes_engage();
    } 
  }
  
  // Receive the current joystic coordinates
  Serial.readBytes((char *) &currentJoystick, 8);
 
  // Do this action
  if(currentJoystick.magnitude==0){
    motors_off();
    if(joystickReleased==false){
      joystickReleaseTime = millis();
      joystickReleased = true;
    }
    if(brakeState == false && millis()-joystickReleaseTime>400){
      brakes_engage();
    }      
  } else {
    joystickReleased = false;  
    joystick_to_ugv(currentJoystick,&desiredUGV);
    desired_to_current_ugv(&desiredUGV,&currentUGV);
    move_ugv(currentUGV);
  }
  lastJoystickCommand = millis();
  
  // TODO Send back sensor data
  Serial.print("currentJoystick now shows: ");
  Serial.print(currentJoystick.angle);
  Serial.print("\t");
  Serial.println(currentJoystick.magnitude);
}

void move_ugv(ugv_state_t ugv){
  // Use "currentState" for ths function
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
  // TODO insert joystick init code here
  currentJoystick.angle = 0;
  currentJoystick.magnitude = 0;
  lastJoystickCommand = millis();
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
  // have a buzzer notification like the mini quads
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
/* By Sam and Austin
NOTE: Forward has been redefined as 0 deg!!!

Angle/quadrant definitons:
             0
             |
        1st  |  4th
             |
+Pi/2 -------+------- -Pi/2
             |
        2nd  |  3rd
             |
           +/- Pi

Left Motor behaviour:
  1st quadrant: speed transitions from full forward to full reverse linearly.
  2nd quadrant: motor runs at full reverse.
  3rd quadrant: speed transitions from full forward to full reverse linearly.
  4th quadrant: motor runs at full forward.

Left Motor behaviour:
  1st quadrant: motor runs at full forward.
  2nd quadrant: speed transitions from full forward to full reverse linearly.
  3rd quadrant: motor runs at full reverse.
  4th quadrant: speed transitions from full forward to full reverse linearly.
*/
void joystick_to_ugv(joystick_t joystick, ugv_state_t *ugv){
  float pi = 3.1415926535; //I think the constant "PI" is already defined in Arudino?
  
  // Setup left and right motor behaviour based on joystick angle.
  // Probably can be simplified somehow. Write a function or two.
  if (joystick.angle >= 0 && joystick.angle < pi/2){
    // QUADRANT ONE
    // forward left
    ugv->rightMotor.direction = FORWARD;
    ugv->rightMotor.speed = 1;
    if (joystick.angle < pi/4){
      ugv->leftMotor.direction = FORWARD;
    } else {
      ugv->leftMotor.direction = REVERSE;
    }
    ugv->leftMotor.speed = abs(joystick.angle-pi/4)/(pi/4);
  } else if (joystick.angle >= pi/2 && joystick.angle <= pi){
    // QUADRANT 2
    // back left
    ugv->leftMotor.direction = REVERSE;
    ugv->leftMotor.speed = 1;
    if (joystick.angle < 3*pi/4){
      ugv->rightMotor.direction = FORWARD;
    } else {
      ugv->rightMotor.direction = REVERSE;
    }
    ugv->rightMotor.speed = abs(joystick.angle-3*pi/4)/(pi/4);
  } else if (joystick.angle >= -pi/2 && joystick.angle < 0){
    // QUADRANT 4
    // forward right
    ugv->leftMotor.direction = FORWARD;
    ugv->leftMotor.speed = 1;
    if (joystick.angle > -pi/4){
      ugv->rightMotor.direction = FORWARD;
    } else {
      ugv->rightMotor.direction = REVERSE;
    }
    ugv->rightMotor.speed = abs(joystick.angle+pi/4)/(pi/4);
  } else  if (joystick.angle >= -pi && joystick.angle < -pi/2){
    // QUADRANT 3
    // back right
    ugv->rightMotor.direction = REVERSE;
    ugv->rightMotor.speed = 1;
    if (joystick.angle > -3*pi/4){
      ugv->leftMotor.direction = FORWARD;
    } else {
      ugv->leftMotor.direction = REVERSE;
    }
    ugv->leftMotor.speed = abs(joystick.angle+3*pi/4)/(pi/4);
  } else {
    //something wierd has happened...
  }
  // Recale output so speed reflects magnitude of jostick displacement.
  // Constain is probably not neccessary.
  //TODO: UNTESTED
  ugv->leftMotor.speed  = constrain(ugv->leftMotor.speed*joystick.magnitude,0,1);
  ugv->rightMotor.speed = constrain(ugv->rightMotor.speed*joystick.magnitude,0,1);
}

