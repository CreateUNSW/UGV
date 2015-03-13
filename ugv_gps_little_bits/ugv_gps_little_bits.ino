/* * * * 
* These are bits and pieces to create the base for GPS controlling of Create's UGV
* We're using TinyGPS+ and Create's compass bearing read to navigate
* We also use Create's car kit motor code to actuate the car's motors and test stuff - this will be replaced
* @version 1.0
* @author Create UGV team
*/

#include <Create.h>
#include <math.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// The GPS using TinyGPSPlus library
TinyGPSPlus gps;
static const int RXPin = 0, TXPin = 1; // using pins 0,1 on the car kit - Arduino UNO
static const uint32_t GPSBaud = 4800;
// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

// Set up the compass sensor
Compass myCompass;
int bearing; // compass sensor reading
double gps_angle; // the target angle to turn to

// The car kit motors
#define POWER 100 // Set power to drive motors for this test,
                  // power ranges from 0-255
// Set up the left motor to be on pins 2 and 3
Motor left(2,3);
// Set up the right motor to be on pins 4 and 5
Motor right(4,5);

void setup(){
  Serial.begin(9600);  // Allow for printing to the computer
  
  // Compass
  myCompass.init();     // Initialize the compass sensor
  
  // GPS
  //Serial.begin(115200);
  ss.begin(GPSBaud);
  
  // car kit motors
  left.init();
  right.init();
  
  delay(500);
}

void loop(){
  // Code below is looped continuously and forever
  
  // GPS test
  // This sketch displays information every time a new sentence is correctly encoded.
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      //displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    //while(true);
  }
  
  // set of target coordinates - different spots on campus - TEMP test data before we make Bluetooth (or otherwise) coordinate sending
  // structure: x, y (x = lng, y = lat)
  double target_coordinates[2][2] = {
    {151.220803, -33.883842},
    {151.220622, -33.884304}
  };
  int where = 1; // temp setting of the target coordinates
  
  // get current location
  double y1 = -33.883876; // y initial = Google Lng
  double x1 = 151.220161; // x initial = Google Lat  
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
    // actual location
    y1 = gps.location.lng();
    x1 = gps.location.lat();
  }
  else
  {
    Serial.print(F("INVALID"));
  }
  
  // where to go - WILL REPLACE when we have the bluetooth module
  double x2 = target_coordinates[where][0];
  double y2 = target_coordinates[where][1];

  // stuff commented out below is NOT NEEDED PROBABLY - Leaving in case we run in problems
  // angle negative from north - left half of the circle
  /*
  if(x1 > x2) {
    //angle = -angle;
  }
  // angle positive from north - right half of the circle
  else {
    // do nothing, it's abs
    //angle = angle);
  }*/
  
  // if the robot is not at the target point yet
  if(!robot_to_point(0.05, x1, y1, x2, y2))
  {
    Serial.print('not there yet');
    boolean off_course = false;
    // get the angle to turn to from inverse tan between the current and target coordinates
    double gps_angle = atan((x2-x1)/(y2-y1)) * 180/PI;
    Serial.print("GPS angle: ");
    Serial.print(gps_angle);
    Serial.println(" degrees");  
    
    // Read from ultrasonic sensor and store in our variable ONCE
    bearing = myCompass.getHeading();
    // offset the bearing to fix for compass error
    bearing = offset_bearing(bearing);
  
    // if we're off the course - May replace this later with a more sophisticated off-the-course check
    if(abs(bearing - gps_angle) > 3) // angle is off?
    {
      off_course = true;
      // keep rotating till the course is fixed - need a better solution
      while(off_course)
      {
        // keep reading from ultrasonic sensor and storing in our variable
        bearing = myCompass.getHeading();
        // offset the bearing to fix for compass error
        bearing = offset_bearing(bearing);
        if(abs(bearing - gps_angle) < 3)
        {
          off_course = false;
        }      
        // Print out the data to via serial to the computer
        //Serial.print("Bearing is:   ");
        //Serial.print(bearing);
        //Serial.println(" degrees");

        // if anticlockwise rotation is required
        if(rotation_dir(bearing, gps_angle))
        {
          // rotate
          aclw();
        }
        // otherwise - clockwise
        else
        {
          // rotate
          clw();
        }
      }
      // Turn both motors off
      left.off();
      right.off();
    }
    // we're not off the course - let's drive
    else// if(!robot_in_danger()) - check if we don't have obstructions...
    {
      forward();
    }
  }  

  // wait for a bit
  delay(1000);  
}

/* * * * 
* Temporary function to rotate clockwise - works for car kit, should be modified for the UGV
*/
void clw()
{
    // Spin clockwise for one second
    left.drive(FWD,POWER);
    right.drive(REV,POWER);
}

/* * * * 
* Temporary function to rotate anitclockwise - works for car kit, should be modified for the UGV
*/
void aclw()
{
    // Spin anticlockwise for one second
    left.drive(REV,POWER);
    right.drive(FWD,POWER);
}

/* * * * 
* Temporary function to drive forward
*/
void forward()
{
    // Spin clockwise for one second
    left.drive(FWD,POWER);
    right.drive(FWD,POWER);
}

/* * * * 
* Offsetting the bearing by the temp value - it depends on the compass we use... - a bit dodgy!
*/
double offset_bearing(int curr_bearing)
{
  int offset = 37; // the offset for this compass is 37 degrees
  curr_bearing = curr_bearing - offset;
  if(curr_bearing < -180)
  {
    curr_bearing = 180 - abs(curr_bearing - 180);
  }
  return curr_bearing;
}

/* * * * 
* Rotation direction choice algorhytm
* Checks the angle from the compass and makes the decision what is the shortest way to rotate to that: CW or CCW
* @return 1 - CCW, 0 - CW
*/
int rotation_dir(int curr, int target)
{
  // get opposite from end
  double opposite;
  if(target <= 0) {
    opposite = target + 180;
  }
  else {
    opposite = target - 180;
  }
  if(target > 0) {
    if(bearing > target || bearing < opposite) {
      // turn anticlockwise
      return 1;
    }
    else {
      // turn clockwise
      return 0;
    }
  }
  else {
    if(bearing < target || bearing > opposite) {
      // turn clockwise
      return 0;
    }
    else {
      // turn anticlockwise
      return 1;
    }
  }  
}


// this checks if the robot is in the right spot
boolean robot_to_point(double radi, double x1, double y1, double x2, double y2)
{
  double tempposx, tempposy, tempposz;
  if(x1 && y1 && x2 && y2)
  {
    tempposx = (x1 - x2);
    tempposy = (y1 - y2);
    return 0; // COORDINATES DO NOT WORK YET
    // check if the old position is within the range
    if (((tempposx < radi) && (tempposx > -radi)) && ((tempposy < radi) && (tempposy > -radi)) && ((tempposz < radi) && (tempposz > -radi)))
    {          
      return 1;
    }
  }
  return 0;
}
