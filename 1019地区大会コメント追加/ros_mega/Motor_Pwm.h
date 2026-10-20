#ifndef MOTOR_PWM_H
#define MOTOR_PWM_H

#include <Arduino.h>


//=====
// DCモータクラス
//=====

class Motor
{
private:

    //===== ピン =====
    uint8_t pwmPin;
    uint8_t dirPin;


public:

    //===== コンストラクタ =====
    Motor(uint8_t pwmPin, uint8_t dirPin);

    //===== 初期化 =====
    void begin();

    //===== モータ駆動 =====    speed = -255~255
    void drive(int speed);  

    //===== 停止 =====
    void stop();

};

#endif