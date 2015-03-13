/*
This motor driving example was written by CREATE UNSW.
The motors should be driven from pins 2, 3, 4 and 5 so
that we may utilize the Arduino's PWM enabled pins on
3 and 5.

An instance of each motor is created as a global construct
on their respective pins at the start of the program. The
Arduino pins are set up for output in the "init" function.

The "drive" function takes in a direction either FWD or REV,
as well as a power between 0 and 255, with 255 being maximum
power. The "off" function turns off the motor.

To use a motor's function, simply write the name of the motor,
then a ".", and then the function you want to call.
*/

#include <Create.h>

#define POWER 100 // Set power to drive motors for this test,
                  // power ranges from 0-255

// Set up the left motor to be on pins 2 and 3
Motor left(2,3);
// Set up the right motor to be on pins 4 and 5
Motor right(4,5);

void setup(){
  // Initialize both motors
  left.init();
  right.init();
}

void loop(){
  
  // Drive both wheels forward for two seconds
  left.drive(FWD,POWER);
  right.drive(FWD,POWER);
  delay(2000);
  
  // Drive both wheels backward for two seconds
  left.drive(REV,POWER);
  right.drive(REV,POWER);
  delay(2000);
  
  // Turn both motors off for one second
  left.off();
  right.off();
  delay(1000);
  
  // Spin anticlockwise for one second
  left.drive(REV,POWER);
  right.drive(FWD,POWER);
  delay(1000);
  
  // Spin clockwise for one second
  left.drive(FWD,POWER);
  right.drive(REV,POWER);
  delay(1000);
  
}
