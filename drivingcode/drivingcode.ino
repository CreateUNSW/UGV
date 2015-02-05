// Arduino Electric Wheelchair Interface
// CREATE UGV TEAM Mk I
// 05 February, 2015
// Original written by Austin Kong
// Modified by Nathan Adler
// Further modified by Yunzhen Zhang (Winnie)

//Controls:
//Drive with 'W', 'S', 'A', 'D'
//'X' is the kill switch. Instantly shuts off all motors

#include <SPI.h>         // Remember this line!
#include <DAC_MCP49xx.h>

// The Arduino pin used for the slave select / chip select on DAC
// Forward/reverse
#define SS_PIN1 2
// Left right
#define SS_PIN2 3

//12bit DAC with 0 to 5V full scale. Set for 1-4V
#define MIN_VOLT 819
#define CENTRE_VOLT 2048
#define MAX_VOLT 3276

#define MIN_SPEED 0
#define MAX_SPEED 4

//Minimum time for the motors to stay active before they are allowed to shut down
//As the solenoid brakes need some time.
#define MIN_RUN_TIME 600
//Just a fudge factor as my keyboard doesn't send keystrokes fast enough
//so add an artifical wait timer to prevent code to go into slow dow mode
#define MIN_UPDATE_TIME 50

// Set up the DAC. 
// First argument: DAC model (MCP4901, MCP4911, MCP4921)
// Second argument: SS pin (10 is preferred)
// (The third argument, the LDAC pin, can be left out if not used)
DAC_MCP49xx dac1(DAC_MCP49xx::MCP4921, SS_PIN1);
DAC_MCP49xx dac2(DAC_MCP49xx::MCP4921, SS_PIN2);

int udSpeed = CENTRE_VOLT;
int lrSpeed = CENTRE_VOLT;

char in;
char dir;
int speeed = 2;

void setup() {
  // Set the SPI frequency to 1 MHz (on 16 MHz Arduinos), to be safe.
  // DIV2 = 8 MHz works for me, though, even on a breadboard.
  // This is not strictly required, as there is a default setting.
  // Use "port writes", see the manual page. In short, if you use pin 10 for
  // SS (and pin 7 for LDAC, if used), this is much faster.
  // Also not strictly required (no setup() code is needed at all).
  
  // Up/Down -- Fwd/Rev
  dac1.setSPIDivider(SPI_CLOCK_DIV2);
  dac1.setPortWrite(false);
  dac1.output(2048);//Initalise to output 2.5V to ensure it passes wheelchair self check  
  // Left/Right
  dac2.setSPIDivider(SPI_CLOCK_DIV2);
  dac2.setPortWrite(false);
  dac2.output(2048);//Initalise to output 2.5V to ensure it passes wheelchair self check
  
}

// Output something slow enough that a multimeter can pick it up.
// For MCP4901, use values below (but including) 255.
// For MCP4911, use values below (but including) 1023.
// For MCP4921, use values below (but including) 4095.
void loop() {
  if (Serial.available()) {
    in = Serial.read();
    switch (in) {
      case 'w'://Forward
        dir = in;
        udSpeed = map(speeed, MIN_SPEED, MAX_SPEED, CENTRE_VOLT, MAX_VOLT);
        lrSpeed = CENTRE_VOLT;
        break;
      case 's'://Reverse
        dir = in;
        udSpeed = map(speeed, MIN_SPEED, MAX_SPEED, CENTRE_VOLT, MIN_VOLT);
        lrSpeed = CENTRE_VOLT;
        break;
      case 'a'://Left
        dir = in;
        break;
      case 'd'://Right
        dir = in;
        break;
      case 'x':
        //Kill Switch
        udSpeed = CENTRE_VOLT;
        lrSpeed = CENTRE_VOLT;
        break;
      case '.':
        // velocity up
        speeed = constrain(speeed+1, MIN_SPEED, MAX_SPEED);
        in = dir;
        break;
      case ',':
        //velocity down
        speeed = constrain(speeed-1, MIN_SPEED, MAX_SPEED);
        break;
      default:
        // Other characters will "hold" the velocity
        // As releasing all keys will start to slow it down
        break;
    }
    /*analogWrite(ud, udSpeed);14
    analogWrite(lr, lrSpeed);
    Serial.println(in);
    Serial.print("ud ");
    Serial.println(udSpeed);
    Serial.print("lr ");
    Serial.println(lrSpeed);*/
    //delay(100);
  }
 
 
  
  //Write speed to output
  dac1.output(udSpeed);
  dac2.output(lrSpeed);
  
  /*float z = 0;
  for (z = 0; z < 360; z++) {
    polar2XY(1024, (3.1415/180)*0);
    delay(10);
  }*/
 
}


void polar2XY (float radius, float angle) {
  int x, y;
  x = radius*cos(angle)+2048;
  y = radius*sin(angle)+2048;
  Serial.println("X");
  Serial.println(x);
  Serial.println("Y");
  Serial.println(y);
  dac1.output(x);
  dac2.output(y);
}
