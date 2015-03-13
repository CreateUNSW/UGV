#include "Create.h"

Motor::Motor(int _pin1, int _pin2){
    //assert(_pin1>=2&&_pin1<=5);
    //assert(_pin2>=2&&_pin2<=5);
    pin1 = _pin1;
    pin2 = _pin2;

}

void Motor::init(){
	pinMode(pin1, OUTPUT);
	pinMode(pin2, OUTPUT);
	off();
}

void Motor::drive(int _dir,int _pwr){
    _pwr = max(0,_pwr);
    _pwr = min(255,_pwr);
    if(_dir==FWD){
        digitalWrite(pin1,HIGH);
        analogWrite(pin2,255-_pwr);
    } else {
        digitalWrite(pin1,LOW);
        analogWrite(pin2,_pwr);
    }
}

void Motor::off(){
	digitalWrite(pin1, LOW);
	digitalWrite(pin2, LOW);
}

Ultrasonic::Ultrasonic(int _trig, int _echo): NewPing(_trig, _echo, MAX_D){
	trig = _trig;
	echo = _echo;
}

void Ultrasonic::init(){
    //Do nothing
}

int Ultrasonic::ping_cm(){
    int distance = NewPing::ping_cm();
    if(distance==NO_ECHO)
        return MAX_D;
    else
        return distance;
}

Compass::Compass():HMC5883L(){
}

void Compass::init(){
	Wire.begin();
	HMC5883L::initialize();
}

int Compass::getHeading(){
	HMC5883L::getHeading(&mx, &my, &mz);
	heading = atan2(my, mx);
	//if (heading < 0)
	//	heading += 2 * M_PI;
	return (int)(heading * 180 / M_PI);
}
