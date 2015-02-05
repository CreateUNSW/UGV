// Arduino Electric Wheelchair Interface
// CREATE UGV TEAM Mk I
// 2014 09 26
// Written by Austin
// Modified by Nathan

//Was originally written to have variable velocity/acceleration control over just WSAD keys
//when turning, forward/reverse speed is kept.
//Each keystroke recieved increases velocity and the rate of velocity increase is goverened 
//from the time it started moving
//To hold a velocity less than max, just press a random key
//If no keystrokes are recieved, it goes into shutdown mode and starts to slow down.


//This fancy velocity control thing isn't really needed for the final UGV
//Only implemented as you can't get velocity control WSAD
//Its just something I did because I could.
//TODO
//Write a function that takes in polar coordinates to convert it to the XY joystick output
//Use the TimerOne library or similar to actually time the velocity/acceleration changes
//rather than waiting for keystrokes. More robust.

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

unsigned long lastUpdate = 0;//"Time between strokes"
unsigned long prevTime = 0;//to start of key strokes
//unsigned long dt = 0;
unsigned long dtt = 0;
byte runState = false;//Associated with MIN)RUN_TIME

int udSpeed = CENTRE_VOLT;
int lrSpeed = CENTRE_VOLT;
int updateSpeedRate = 1;//THhis changes as time passes

char in;

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
  
  //Serial.begin(115200);
  //Serial.println("Arduino UGV Interface...");

}

// Output something slow enough that a multimeter can pick it up.
// For MCP4901, use values below (but including) 255.
// For MCP4911, use values below (but including) 1023.
// For MCP4921, use values below (but including) 4095.
void loop() {
  in = '0';
  dtt = (unsigned long)(millis() - prevTime);
  //TODO use timer to physically time how long its been running rather then waiting for keys
  //Can also ramp up this update rate
  if (dtt > 15*MIN_UPDATE_TIME) {
    updateSpeedRate = 25;
  } else if (dtt > 10*MIN_UPDATE_TIME) {
    updateSpeedRate = 15;
  } else if (dtt > 5*MIN_UPDATE_TIME) {
    updateSpeedRate = 10;
  } else if (dtt > 2*MIN_UPDATE_TIME) {
    updateSpeedRate = 5;
  } else {
    updateSpeedRate = 2;
  }
  if (Serial.available()) {
    in = Serial.read();
    lastUpdate = millis();
    if (!runState) {//Activate runState
      runState = true;
      digitalWrite(ct, LOW);
      prevTime = millis();//Reset timer for MIM_RUN_TIME
    }
    switch (in) {
      case 'w'://Forward
        if (udSpeed < MAX_VOLT) {
          udSpeed+=updateSpeedRate;
          if (udSpeed > MAX_VOLT) {
            udSpeed = MAX_VOLT;
          }
        }
        //Straighten back out
        /*if (lrSpeed > CENTRE_VOLT) {
          lrSpeed-=updateSpeedRate;
          if (lrSpeed < CENTRE_VOLT) {
            lrSpeed = CENTRE_VOLT;
          }
        } else if (lrSpeed < CENTRE_VOLT) {
          lrSpeed+=updateSpeedRate;
          if (lrSpeed > CENTRE_VOLT) {
            lrSpeed = CENTRE_VOLT;
          }
        }*/
        //Ignore comment above. keeping it simpler
        lrSpeed = CENTRE_VOLT;
        //analogWrite(ud, MAX_PWM);
        //analogWrite(lr, CENTRE_PWM);
        //dac1.output(MAX_VOLT);
        //dac2.output(CENTRE_VOLT);
        break;
      case 's'://Reverse
        if (udSpeed > MIN_VOLT) {
          udSpeed-=updateSpeedRate;
          if (udSpeed < MIN_VOLT) {
            udSpeed = MIN_VOLT;
          }
        }
        //Straighten back out
        /*if (lrSpeed > CENTRE_VOLT) {
          lrSpeed-=updateSpeedRate;
          if (lrSpeed < CENTRE_VOLT) {
            lrSpeed = CENTRE_VOLT;
          }
        } else if (lrSpeed < CENTRE_VOLT) {
          lrSpeed+=updateSpeedRate;
          if (lrSpeed > CENTRE_VOLT) {
            lrSpeed = CENTRE_VOLT;
          }
        }*/
        lrSpeed = CENTRE_VOLT;
        //analogWrite(ud, MIN_PWM);
        //analogWrite(lr, CENTRE_PWM);
        //dac1.output(MIN_VOLT);
        //dac2.output(CENTRE_VOLT);
        break;
      case 'a'://Left
        if (lrSpeed > MIN_VOLT) {
          lrSpeed-=updateSpeedRate;
          if (lrSpeed < MIN_VOLT) {
            lrSpeed = MIN_VOLT;
          }
        }
        //Ignore comment below. keep it simple
        lrSpeed = CENTRE_VOLT;
        //If turning, dont reduce forward velocity
        /*if (udSpeed > CENTRE_VOLT) {
          udSpeed--;
        } else if (udSpeed < CENTRE_VOLT) {
          udSpeed++;
        }*/
        //analogWrite(ud, CENTRE_PWM);
        //analogWrite(lr, MIN_PWM);
        //dac1.output(CENTRE_VOLT);
        //dac2.output(MIN_VOLT);
        break;
      case 'd'://Right
        if (lrSpeed < MAX_VOLT) {
          lrSpeed+=updateSpeedRate;
          if (lrSpeed > MAX_VOLT) {
            lrSpeed = MAX_VOLT;
          }
        }
        //Ignore comment below. keep it simple
        lrSpeed = CENTRE_VOLT;
        //If turning, dont reduce forward velocity
        /*if (udSpeed > CENTRE_VOLT) {
          udSpeed-=updateSpeedRate;
        } else if (udSpeed < CENTRE_VOLT) {
          udSpeed+=updateSpeedRate;
        }*/
        //analogWrite(ud, CENTRE_PWM);
        //analogWrite(lr, MAX_PWM);
        //dac1.output(CENTRE_VOLT);
        //dac2.output(MAX_VOLT);
        break;
      case 'x':
        //Kill Switch
        udSpeed = CENTRE_VOLT;
        lrSpeed = CENTRE_VOLT;
        break;
      case '.':
        // velocity up
        
        break;
      case ',':
        //velocity down
        
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
  } else if (((unsigned long)(millis() - prevTime) > MIN_RUN_TIME) && ((unsigned long)(millis() - lastUpdate) > MIN_UPDATE_TIME)) {
  //prevents roll over error, although that'll require the sketch to be running for 50 staight days
  //If no character is being sent over, check that it has been running for more then MIN_RUN_TIME (delay of solenoid brakes)
  //and MUM_UPDATE_TIME, how fast my keyboard can send keys
  //Then move into state where it starts to ramp the velocity back down
    lastUpdate = millis();
    if (runState) {//Deactivate runstate
      runState = false;
      digitalWrite(ct, HIGH);
    }
    //Return joystick to central position
    if (udSpeed > CENTRE_VOLT) {
      udSpeed-=updateSpeedRate;
      if (udSpeed < CENTRE_VOLT) {
        udSpeed = CENTRE_VOLT;
      }
    } else if (udSpeed < CENTRE_VOLT) {
      udSpeed+=updateSpeedRate;
      if (udSpeed > CENTRE_VOLT) {
        udSpeed = CENTRE_VOLT;
      }
    }
    if (lrSpeed > CENTRE_VOLT) {
      lrSpeed-=updateSpeedRate;
      if (lrSpeed < CENTRE_VOLT) {
        lrSpeed = CENTRE_VOLT;
      }
    } else if (lrSpeed < CENTRE_VOLT) {
      lrSpeed+=updateSpeedRate;
      if (lrSpeed > CENTRE_VOLT) {
        lrSpeed = CENTRE_VOLT;
      }
    }
    //Diagnostic printout
    /*analogWrite(ud, udSpeed);
    analogWrite(lr, lrSpeed);
    Serial.print("ud ");
    Serial.println(udSpeed);
    Serial.print("lr ");
    Serial.println(lrSpeed);*/
    runState = false;
  }
 
 
  
  //Write speed to output
  //dac1.output(udSpeed);
  //dac2.output(lrSpeed);
  
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
