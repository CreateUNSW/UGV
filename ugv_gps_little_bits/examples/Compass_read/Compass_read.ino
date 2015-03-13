#include <Create.h>

// Set up the compass sensor
Compass myCompass;

// Define a variable that stores data from the
// compass sensor
int bearing;

void setup(){
  // Code below runs at the beginning of the program
  
  Serial.begin(9600);  // Allow for printing to the computer
  myCompass.init();     // Initialize the compass sensor
  
}

void loop(){
  // Code below is looped continuously and forever
  
  // Read from ultrasonic sensor and store in our variable
  bearing = myCompass.getHeading();
  
  // Print out the data to via serial to the computer
  Serial.print("Bearing is:   ");
  Serial.print(bearing);
  Serial.println(" degrees");
  
}
