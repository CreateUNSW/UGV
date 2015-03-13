#ifndef CREATE_H_INCLUDED
#define CREATE_H_INCLUDED

#include "utility/Wire.h"
#include "utility/I2Cdev.h"
#include "utility/HMC5883L.h"
#include "utility/NewPing.h"

#define MAX_D 150

#define FWD 1
#define REV 0

class Motor
{
private:
    int pin1;
    int pin2;
public:
    Motor(int _pin1, int _pin2);
	void init();
    void drive(int _dir, int _pwr);
	void off();
};

class Ultrasonic : public NewPing
{
private:
	uint8_t trig;
	uint8_t echo;
public:
	Ultrasonic(int _trig, int _echo);
	void init();
	int ping_cm();
};

class Compass : public HMC5883L
{
private:
	int16_t mx, my, mz;
	float heading;
public:
	Compass();
	void init();
	int getHeading();
};

#endif // CREATE_H_INCLUDED
