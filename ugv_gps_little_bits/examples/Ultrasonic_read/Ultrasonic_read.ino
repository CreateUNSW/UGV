/*
This example demonstrates interfacing with the ultrasonic sensor.
The distance of an object from the ultrasonic sensor can be read
using the "ping_cm()" function. If there is no obstacle detected
within 150cm of the sensor, the function will return the maximum
value of 150.
*/

#include <Create.h>

#define TRIGGER_PIN 6
#define ECHO_PIN    7

// Set up the ultrasonic sensor on pins 6 & 7
Ultrasonic mySensor(TRIGGER_PIN, ECHO_PIN);

// Define a variable that stores data from the
// ultrasonic sensor
int distance;

void setup(){
  // Code below runs at the beginning of the program
  
  Serial.begin(9600);  // Allow for printing to the computer
  mySensor.init();     // Initialize the ultrasonic sensor
  
}

void loop(){
  // Code below is looped continuously and forever
  
  // Read from ultrasonic sensor and store in our variable
  distance = mySensor.ping_cm();
  
  // Print out the data to via serial to the computer
  Serial.print("Distance measured: ");
  Serial.print(distance);
  Serial.println(" cm");
  
}
