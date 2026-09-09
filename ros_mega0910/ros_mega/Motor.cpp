#include <Arduino.h>
#include "Motor.h"

Motor::Motor(uint8_t pwmPin, uint8_t dirPin)
{
    this->pwmPin = pwmPin;
    this->dirPin = dirPin;
}

void Motor::begin()
{
    pinMode(pwmPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
}

void Motor::drive(int speed)
{
    bool direction = speed >= 0;

    digitalWrite(dirPin, direction);

    speed = abs(speed);

    if(speed > 255)
        speed = 255;

    analogWrite(pwmPin, speed);
}

void Motor::stop()
{
    analogWrite(pwmPin, 0);
};