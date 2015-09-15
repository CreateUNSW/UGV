#include <Arduino.h>
#include "h-bridge_driver.hpp"
#include "shared.hpp"

#define MAX_COMMAND_LENGTH 15 // chars
#define WATCHDOG_TIMEOUT 100 // milliseconds

// Experimental UGV H-bridge code.
// Public Domain, Copyright William Hales 2015


struct command_s
{
	char text[MAX_COMMAND_LENGTH]; // does not include '[' or ']'
	unsigned char len;
	bool ready;
};

void addCharToCommand( command_s * command, char inchar );
void tryCommand( command_s * command );


// Disable checksum checks and print back expected checksums
bool debugMode = false;
// LEDs have inverted control
int LEDred = 7;
int LEDblue = 8;
// Command watchdog
time_t lastCommand; // changed only in tryCommand()

driver_s * bridgeA;
driver_s * bridgeB;
command_s currentCommand;

void setup()
{
	// ######## ATMEGA-328 Specific flags (Arduino Uno) ########
	// Tell the clock prescaler to not divide our 8mHz down to a lower speed
	CLKPR = 0x80;  // Enable change
	CLKPR = 0x00;  // Desired value (no dividing/scaling)
	// Use 1.1V analog voltage reference
	analogReference( INTERNAL );

	pinMode( LEDred, OUTPUT);
	pinMode( LEDblue, OUTPUT);

	bridgeA = newDriver( 10, 9, 2, A5);
	bridgeB = newDriver(  6, 5, 4, A0);
	currentCommand.ready = false;
	currentCommand.len = 0;
	lastCommand = 0;

	Serial.begin(9600);
	Serial.println("Hey there friend :::)  Type \"[hales]\" to enter debug mode");
	digitalWrite( LEDred, HIGH);
}

void loop()
{
	time_t now = millis();

	if ( Serial.available() )
	{
		char inchar = Serial.read();
		addCharToCommand( &currentCommand, inchar);
		if ( debugMode ) Serial.print( inchar ); // echo back
	}
	if ( currentCommand.ready  )
	{
		tryCommand( &currentCommand );
	}
	
	if ( !debugMode && now >= lastCommand + WATCHDOG_TIMEOUT )
	{
		// We have not recieved commands in a while
		setDriverTargets( bridgeA, FORWARD, 0 );
		setDriverTargets( bridgeB, FORWARD, 0 );
		digitalWrite( LEDred,  LOW  );
		digitalWrite( LEDblue, HIGH );
	}
	else
	{
		digitalWrite( LEDred,  HIGH );
		digitalWrite( LEDblue, LOW  );
	}

	driverTick( bridgeA );
	driverTick( bridgeB );
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
				setDriverTargets( bridgeA, numberAsign, numberA);
				setDriverTargets( bridgeB, numberBsign, numberB);
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
			Serial.print("Bridge A: ");
			Serial.print( getDriverCurrent( bridgeA) );
			Serial.print(" Bridge B: ");
			Serial.println( getDriverCurrent( bridgeB) );
		}
	}


	command->len = 0;
	command->ready = false;
}
