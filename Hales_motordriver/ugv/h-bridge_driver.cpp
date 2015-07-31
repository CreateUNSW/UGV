#include "h-bridge_driver.hpp"
#include <Arduino.h>

#define ADC_SAMPLES 200  // multiple are taken for averaging
#define ADC_INTERVAL 13000 // microseconds

enum driverState_e
{
	DS_RUN,
	DS_HIGHSIDE_INFLUX,
	DS_OCTRIP,
};

struct driver_s
{
	// Mosfet driving
	int pinF;
	int pinB;
	// High-side relay
	int pinDirection;
	// Current sense shunt
	int pinShunt;

	driverState_e state;
	time_t timeEnteredState; // time this current state was entered

	bool curDirection;
	bool targetDirection;
	unsigned char targetPWM;

	// Current monitoring
	unsigned int ADCreadings[ADC_SAMPLES];
	unsigned int lastADCreadingPos;
	time_t lastADCreadingTime; // allowed to overflow every 70 mins (microseconds)
};

driver_s * newDriver( int pinF, int pinB, int pinDirection, int pinShunt)
{
	driver_s * d = (driver_s *)malloc( sizeof(driver_s) );
	if ( d == NULL ) return NULL;

	d->pinF = pinF;
	d->pinB = pinB;
	d->pinDirection = pinDirection;
	d->pinShunt = pinShunt;

	d->state = DS_RUN;
	d->timeEnteredState = millis();
	d->targetPWM = 0;
	d->targetDirection = FORWARD;
	d->curDirection = FORWARD;

	for (int i=0; i<ADC_SAMPLES; i++)
		d->ADCreadings[i] = 0;
	d->lastADCreadingPos = 0;
	d->lastADCreadingTime = micros();

	pinMode( pinF, OUTPUT );
	pinMode( pinB, OUTPUT );
	pinMode( pinDirection, OUTPUT );

	return d;
}

void setDriverTargets( driver_s * d, bool direction, unsigned char pwm)
{
	d->targetPWM = pwm;
	d->targetDirection = direction;
}

void driverTick( driver_s * d)
{
	time_t now = millis();

	// Update current readings
	// WARNING ERROR: FAILS AFTER 70 MINUTES
	time_t now_micros = micros();
	if ( d->lastADCreadingTime + ADC_INTERVAL <= now_micros )
	{
		d->lastADCreadingPos = (d->lastADCreadingPos + 1) % ADC_SAMPLES;
		d->ADCreadings[d->lastADCreadingPos] = analogRead( d->pinShunt );
		d->lastADCreadingTime = now_micros;
	}

	switch ( d->state )
	{
		case DS_RUN:
			if ( getDriverCurrent(d) >= CURRENT_TRIPPOINT )
			{
				//digitalWrite( d->pinF, 0 );
				//digitalWrite( d->pinB, 0 );
				//d->state = DS_OCTRIP;
				//d->timeEnteredState = now;
			}
			else if ( d->curDirection != d->targetDirection )
			{
				digitalWrite( d->pinF, 0 );
				digitalWrite( d->pinB, 0 );
				digitalWrite( d->pinDirection, d->targetDirection );
				d->curDirection = d->targetDirection;
				d->state = DS_HIGHSIDE_INFLUX;
				d->timeEnteredState = now;
			}
			else
			{
				if ( d->curDirection == FORWARD )
				{
					analogWrite( d->pinF, d->targetPWM);
					analogWrite( d->pinB, 0);
				}
				else
				{
					analogWrite( d->pinF, 0);
					analogWrite( d->pinB, d->targetPWM);
				}
			}
			break;

		case DS_HIGHSIDE_INFLUX:
			if ( now > d->timeEnteredState + RELAY_SWITCH_TIME )
			{
				d->state = DS_RUN;
				d->timeEnteredState = now;
			}
			break;

		default:
			// Do nothing
			digitalWrite( d->pinF, 0 );
			digitalWrite( d->pinB, 0 );
			break;
	}
}

unsigned long int getDriverCurrent( driver_s * d)
{
	unsigned long int sum = 0;
	for (int i=0; i<ADC_SAMPLES; i++)
		sum += d->ADCreadings[i];

	//return sum / ADC_SAMPLES;

	unsigned long int top = (unsigned long int)( sum*1074 );
	unsigned long int bot = (unsigned long int)((unsigned long int)SHUNT_RESISTANCE * (unsigned long int)ADC_SAMPLES);

	//Serial.print("(TOP ");
	//Serial.print(top);
	//Serial.print(" BOT ");
	//Serial.print(bot);
	//Serial.println(")");

	return (unsigned long int)(top/bot);
}












