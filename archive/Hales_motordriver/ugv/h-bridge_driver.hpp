#pragma once


#define RELAY_SWITCH_TIME 50 // milliseconds
#define SHUNT_RESISTANCE 50 // milliohms
#define CURRENT_TRIPPOINT 99999999 // milliamps

// Directions
#define FORWARD 0
#define BACKWARD 1

typedef unsigned long time_t;

struct driver_s;


// See cpp file for pin info
driver_s * newDriver( int pinF, int pinB, int pinDirection, int pinShunt);
// Set target speed and direction
void setDriverTargets( driver_s * d, bool direction, unsigned char pwm);
// Update speed, direction in real life and check watchdog timeout
void driverTick( driver_s * d);
// Get averaged current (mA)
unsigned long int getDriverCurrent( driver_s * d);


