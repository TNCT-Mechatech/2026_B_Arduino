#ifndef OMNI_H
#define OMNI_H

#include <Arduino.h>

#include "Motor.h"


//=====
// オムニ制御クラス
//=====

class Omni
{
private:

    //==================================================
    // オムニモータ
    //==================================================

    Motor motor1;
    Motor motor2;
    Motor motor3;
    Motor motor4;


    //==================================================
    // オムニ制御
    //==================================================

    float omega;
    float r;
    float theta;

    float ROT_GAIN;

    //感度調整
    float MOVE_CURVE;
    float ROT_CURVE;


    //==================================================
    // 入力カーブ
    //==================================================

    float curveInput(
        float input,
        float exponent
    );


    //==================================================
    // オムニモータ drive
    //==================================================

    void drive(
        int out1,
        int out2,
        int out3,
        int out4
    );


public:

    //==================================================
    // コンストラクタ
    //==================================================

    Omni(
        uint8_t pwmPin1,
        uint8_t dirPin1,

        uint8_t pwmPin2,
        uint8_t dirPin2,

        uint8_t pwmPin3,
        uint8_t dirPin3,

        uint8_t pwmPin4,
        uint8_t dirPin4
    );


    //==================================================
    // 初期化
    //==================================================

    void begin();


    //==================================================
    // ジョイスティック入力
    //==================================================

    void update(
        int joyX,
        int joyY,
        int joyRot
    );
};

#endif