#include <TimerOne.h>
#include <Servo.h>
#include "shared.hpp"

#define MAX_COMMAND_LENGTH 15 // chars
#define WATCHDOG_TIMEOUT 500 // milliseconds

#define LEFT_MOTOR_PIN 2
#define RIGHT_MOTOR_PIN 3

#define MOTOR_LOW_MICROS 1000
#define MOTOR_HIGH_MICROS 2000
#define MOTOR_OFF_MICROS 1500

/**
 * Smooth rate:
 * Rate of change of control signal to motor.
 * Change in microseconds of signal per second.
 * A rate of MOTOR_HIGH_MICROS-MOTOR_OFF_MICROS will go from full speed to stop in one second
 */
#define SMOOTH_RATE (MOTOR_HIGH_MICROS-MOTOR_OFF_MICROS)

#define SMOOTH_UPDATE_RATE 10 // rate of ISR update in Hz

#define SMOOTH_STEP (SMOOTH_RATE/SMOOTH_UPDATE_RATE)

// Experimental UGV H-bridge code.
// Based on driver by William Hales
// Written by Nathan Adler
// Last edited 3/10/2015

typedef uint32_t time_t;

struct command_s
{
  char text[MAX_COMMAND_LENGTH]; // does not include '[' or ']'
  unsigned char len;
  bool ready;
};

class ugv_motor_t {
  public:
    ugv_motor_t(uint8_t pin){
      motor.attach(pin);
      currentMicros = MOTOR_OFF_MICROS;
      desiredMicros = MOTOR_OFF_MICROS;
    };
    void drive(char sign, int power){
      if(sign!='-'&&sign!='+'){
        return;
      }
      if(sign=='-'){
        power=-power;
      }
      desiredMicros = map(power,-255,255,MOTOR_HIGH_MICROS,MOTOR_LOW_MICROS);
      //motor.writeMicroseconds(desiredMicros);
    }
    void stop(){
       motor.writeMicroseconds(MOTOR_OFF_MICROS);
       desiredMicros = MOTOR_OFF_MICROS;
       currentMicros = MOTOR_OFF_MICROS;
    }
    void smooth(){
      if(currentMicros==desiredMicros){
        // do nothing
      } else if(abs(currentMicros-desiredMicros)<=SMOOTH_STEP){
        motor.writeMicroseconds(desiredMicros);
        currentMicros = desiredMicros;
      } else {
        currentMicros += (desiredMicros>currentMicros) ? SMOOTH_STEP : -SMOOTH_STEP;
        motor.writeMicroseconds(currentMicros);
      }
    }
  private:
    Servo motor;
    int currentMicros;
    int desiredMicros;
};

void addCharToCommand( command_s * command, char inchar );
void tryCommand( command_s * command );


// Disable checksum checks and print back expected checksums
bool debugMode = false;
int LEDblue = 13;
// Command watchdog
time_t lastCommand; // changed only in tryCommand()

command_s currentCommand;

ugv_motor_t leftMotor(LEFT_MOTOR_PIN);
ugv_motor_t rightMotor(RIGHT_MOTOR_PIN);

void motor_smooth_ISR(){
  leftMotor.smooth();
  rightMotor.smooth();
}

void setup()
{
  pinMode( LEDblue, OUTPUT);
  currentCommand.ready = false;
  currentCommand.len = 0;
  lastCommand = 0;

  Serial.begin(9600);
  //Serial.println("Hey there friend :::)  Type \"[hales]\" to enter debug mode");
  Timer1.initialize(1000000/SMOOTH_UPDATE_RATE);
  Timer1.attachInterrupt(motor_smooth_ISR);
  
}

void loop()
{
  time_t now = millis();

  if ( Serial.available() )
  {
    char inchar = Serial.read();
    addCharToCommand( &currentCommand, inchar);
   // if ( debugMode ) Serial.print( inchar ); // echo back: !!! Warning! Breaks ros node as of 16/9/2015
  }
  if ( currentCommand.ready  )
  {
    tryCommand( &currentCommand );
  }
  
  if ( !debugMode && now >= lastCommand + WATCHDOG_TIMEOUT )
  {
    // We have not recieved commands in a while
    leftMotor.stop();
    rightMotor.stop();
    digitalWrite( LEDblue, HIGH );
  }
  else
  {
    digitalWrite( LEDblue, LOW  );
  }
}

void addCharToCommand( command_s * command, char inchar )
{
  if (  inchar == '[' )
  {
    // Start a new command
    command->len = 0;
  }
  else if ( inchar == ']' )
  {
    // End of command
    command->ready = true;
  }
  else if ( command->len >= MAX_COMMAND_LENGTH )
  {
    // Start a new command
    command->text[0] = inchar;
    command->len = 1;
  }
  else
  {
    // Append char
    command->text[command->len] = inchar;
    command->len++;
  }
}

void tryCommand( command_s * command )
{
  // Normal motor direction and speed command
  // Format is [SAADBB:CC]
  //     S = direction of bridge A, either '+' or '-'
  //    AA = PWM of bridge A, from 00 to FF
  //     D = direction of bridge B, either '+' or '-
  //    BB = PWM of bridge A, from 00 to FF
  //     : = a literal ':' must be here
  //    CC = checksum of whole command
  if ( command->len == 9 )
  {
    if ( ( command->text[0] == '-' || command->text[0] == '+' ) && ( command->text[3] == '-' || command->text[3] == '+' ) && ( command->text[6] == ':' ) )
    {
      bool numberAsign = ( command->text[0] == '-' );
      unsigned char numberA  = hexToNum( command->text[1] ) * 16 + hexToNum( command->text[2] );
      bool numberBsign = ( command->text[3] == '-' );
      unsigned char numberB  = hexToNum( command->text[4] ) * 16 + hexToNum( command->text[5] );
      unsigned char claimedChecksum = hexToNum( command->text[7] ) * 16 + hexToNum( command->text[8] );

      unsigned char actualChecksum = calculateChecksum( command->text, command->len );

      if ( debugMode )
      {
        Serial.print(" Expected checksum ");
        Serial.print(   decToHex( actualChecksum / 16 ) );
        Serial.println( decToHex( actualChecksum % 16 ) );
      }

      if ( actualChecksum == claimedChecksum || debugMode )
      {
        //setDriverTargets( bridgeA, numberAsign, numberA);
        //setDriverTargets( bridgeB, numberBsign, numberB);
        leftMotor.drive(command->text[0],(int)numberA);
        rightMotor.drive(command->text[3],(int)numberB);
        lastCommand = millis();
      }
    }
  }
  // Special command: enter debugging mode.  Checksums are laxed
  else if (command->len == 5)
  {
    char targetmessage[] = "hales";
    bool failed = false;
    for (unsigned char i=0; i<5; i++)
    {
      if ( targetmessage[i] != command->text[i] )
      {
        failed = true;
        break;
      }
    }

    if (!failed)
    {
      debugMode = true;
      Serial.println("Entered debug mode");
    }
  }
  // Special command: report current consumption averages
  else if (command->len == 7)
  {
    char targetmessage[] = "current";
    bool failed = false;
    for (unsigned char i=0; i<7; i++)
    {
      if ( targetmessage[i] != command->text[i] )
      {
        failed = true;
        break;
      }
    }

    if (!failed)
    {
      /*Serial.print("Bridge A: ");
      Serial.print( getDriverCurrent( bridgeA) );
      Serial.print(" Bridge B: ");
      Serial.println( getDriverCurrent( bridgeB) );*/
      //Serial.println("something failed");
    }
  }
  command->len = 0;
  command->ready = false;
}
