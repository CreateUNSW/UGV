/*
Template for solving the navigation challenge.
*/

#include <Create.h>

#define CLOSE 40

Motor left(2,3);
Motor right(4,5);
Ultrasonic radar(6,7);
Compass north;

int bearing;
int distance;

void setup(){
  left.init();
  right.init();
  radar.init();
  north.init();
  
  // Just in case you want to see the readings
  Serial.begin(9600);
}

void loop(){
  distance = radar.ping_cm();
  bearing = north.getHeading();
  
  if(distance<CLOSE){
    // Robot sees obstacle
    if(bearing<0){
      // Robot is pointing left
    } else {
      // Robot is pointing right
    }
  } else {
    // Robot sees no obstacle
    
  }
}
