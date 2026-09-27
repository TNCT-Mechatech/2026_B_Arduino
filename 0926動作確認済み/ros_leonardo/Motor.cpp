#include <Arduino.h>
#include "Motor.h"

//=====
// コンストラクタ
//=====
Motor::Motor(uint8_t pwmPin, uint8_t dirPin)
{
    this->pwmPin = pwmPin;
    this->dirPin = dirPin;
}

//=====
// 初期化
//=====
void Motor::begin()
{
    pinMode(pwmPin, OUTPUT);

    pinMode(dirPin, OUTPUT);

    stop();
}

//=====
// モータ駆動
//=====
void Motor::drive(int speed)
{
    //===== 方向 =====
    bool direction =
        speed >= 0;

    digitalWrite(dirPin, direction);

    //===== PWM =====
      speed = abs(speed);

    //===== PWM上限 =====
    if(speed > 255)
    {
        speed = 255;
    }

    analogWrite(pwmPin, speed);
}


//=====
// 停止
//=====
void Motor::stop()
{
    analogWrite(pwmPin, 0);
}