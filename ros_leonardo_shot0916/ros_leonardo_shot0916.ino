//=====
// Leonardo
//
// Raspberry Pi → Leonardo
// 足回り
// joyX     : int16_t
// joyY     : int16_t
// joyRot   : int16_t
// limitShootUP  : uint8_t
// limitShootDOWN  : uint8_t
//
// 射出
// buttonL1  : uint8_t  発射
// buttonY   : uint8_t  射出モード切り替え
//
// Leonardo → Raspberry Pi
// buttonL1    : uint8_t
// buttonY     : uint8_t
// 
//
// 合計
// Pi → Leonardo : 8Byte
// Leonardo → Pi : 4Byte
//
//=====
// 0916試走会
// 十字キー移動の実装確認まだ

#include <Arduino.h>

#include "Motor.h"
#include "Omni.h"
#include "TppSol.h"

#include <SerialBridge.hpp>
#include <InoHardwareSerial.hpp>


//=====
// Raspberry Piと同じ構造体
//=====

typedef struct
{
    int16_t joyX;
    int16_t joyY;
    int16_t joyRot;

    uint8_t buttonL1;
    uint8_t buttonY;

} JoyData_t;


//=====
// Leonardo → Raspberry Pi
//=====

typedef struct
{
    uint8_t buttonL1;
    uint8_t buttonY;

    uint8_t limitShootUP;
    uint8_t limitShootDOWN;

} ResponseData_t;


//=====
// SerialBridge
//=====

sb::Message<JoyData_t> joy_msg;

sb::Message<ResponseData_t> response_msg;

InoHardwareSerial serial(&Serial);

SerialBridge bridge(&serial);


//=====
// オムニ
//=====
//
// PWM / DIR
//
// Motor1 : PWM 13  / DIR 12
// Motor2 : PWM 11  / DIR 10
// Motor3 : PWM 9  / DIR 8
// Motor4 : PWM 6 / DIR 7
//
//=====

Omni omni(
    13,  12,
    11,  10,
    9,   8,
    6,   7
);


//=====
// 間欠歯車DCmotor
//=====
const int pwmPin5 = 5;
const int dirPin5 = 4;

//=====
// Tppソレノイド
//=====
const int pwmPinSol = 3;
const int dirPinSol = 2;

//=====
// リミットスイッチ
//=====
// Ls_shoot_UP射出機構＿角度変更の上
// Ls_shoot_DWON 射出機構の_角度変更の下
//
// Ls_catch 射出機構の補充地点
//Ls_shoot = -1; //射出ソレノイド
//=====
const int Ls_shoot_UP = A0; //L1 >> Megaへ
const int Ls_shoot_DOWN = A1; //L2 >> Megaへ

const int Ls_catch = A2; //L3

//=====
// LED
//=====

const int LED_STARTUP = A3;
const int LED_RX = A4;
//const int LED_3 = A2; // 


//=====
// Motor
//=====

Motor ShootMotor(pwmPin5, dirPin5);

//=====
//TppSol
//=====
//(PWMpin, DirPin, 通常50ms, ロング8000ms, インターロック200ms)
TppSol handSol(pwmPinSol, dirPinSol, 50, 8000, 200);



//=====
// 射出機構の状態
//=====

enum ShootState
{
    Shoot_SAFE,
    Shoot_Moving,
    Shoot_STOP_PUSH,
    Shoot_SHOOT_WAIT,
    Shoot_PUSH_LONG,
    /*
    SAFE = 初期状態・安全
    Moving = 間欠ギア（正回転）PULL
    Stop　＝　(停止, PUSH)
    Shoot = (停止, 発射入力待ち)
    PUSH-long = (正回転)
    */
};

ShootState nowShootState = Shoot_SAFE;
unsigned long stateStartTime = 0;

//発射機構の状態機械
void changeShootState(ShootState newState)
{
    nowShootState = newState;
    stateStartTime = millis();
}
//=====
// コントローラー入力
//=====

int joyX = 0;
int joyY = 0;
int joyRot = 0;

uint8_t buttonL1 = 0;
uint8_t buttonY  = 0;

uint8_t oldButtonY = 0;

bool buttonY_pressed = false;


//=====
// setup
//=====

void setup()
{
    //=====
    // LED初期化
    //=====
    pinMode(
        LED_STARTUP,
        OUTPUT
    );
    pinMode(
        LED_RX,
        OUTPUT
    );
    digitalWrite(
        LED_STARTUP,
        LOW
    );
    digitalWrite(
        LED_RX,
        LOW
    );


    //=====
    // Raspberry Pi通信
    //=====

    Serial.begin(115200);


    //=====
    // Pi → Leonardo
    //=====

    bridge.add_frame(
        0,
        &joy_msg
    );


    //=====
    // Leonardo → Pi
    //=====

    bridge.add_frame(
        1,
        &response_msg
    );


    //=====
    // オムニ
    //=====

    omni.begin();


    //=====
    // Motor
    //=====

    ShootMotor.begin();

    // TppSol
    handSol.begin();


    //=====
    // リミットスイッチ
    //=====
    pinMode(Ls_shoot_UP, INPUT_PULLUP); //L1
    pinMode(Ls_shoot_DOWN, INPUT_PULLUP); //L2

    pinMode(Ls_catch, INPUT_PULLUP); //L3

    //=====
    //初期状態
    
    nowShootState = Shoot_SAFE;


    //=====
    // 初期値
    //=====

    joy_msg.data.joyX = 0;
    joy_msg.data.joyY = 0;
    joy_msg.data.joyRot = 0;

    joy_msg.data.buttonL1 = 0;
    joy_msg.data.buttonY = 0;


    response_msg.data.buttonL1 = 0;
    response_msg.data.buttonY = 0;

    response_msg.data.limitShootUP = 0;
    response_msg.data.limitShootDOWN = 0;


    //=====
    // 起動確認
    //=====

    digitalWrite(LED_STARTUP, HIGH);
}


//=====
// loop
//=====

void loop()
{
    //=====
    // Raspberry Piから受信
    //=====

    bridge.update();

    //=====
    // 新しいデータを受信
    //=====

    if (joy_msg.was_updated())
    {
        //=====
        // コントローラー入力
        //=====

        joyX =
            joy_msg.data.joyX;

        joyY =
            joy_msg.data.joyY;

        joyRot =
            joy_msg.data.joyRot;

        buttonL1 =
            joy_msg.data.buttonL1;

        buttonY =
            joy_msg.data.buttonY;


        //=====
        // Yボタンの立ち上がり検出
        //=====

        buttonY_pressed =
            (
                buttonY == 1 &&
                oldButtonY == 0
            );

        oldButtonY = buttonY;


        //=====
        // Raspberry Piへ返信
        //=====

        response_msg.data.buttonL1 =
            buttonL1;

        response_msg.data.buttonY =
            buttonY;
        
        //Ls_shoot_UP >> Mega
        response_msg.data.limitShootUP =
        !digitalRead(Ls_shoot_UP);

        //Ls_shoot_DWON >> Mega
        response_msg.data.limitShootDOWN =
        !digitalRead(Ls_shoot_DOWN);


        bridge.write(1);
    }

    //各機構の状態の更新
    //=====
    // オムニ
    //=====
    omni.update(
        joyX,
        joyY,
        joyRot
    );
    //TppSol
    handSol.update();


    //=====
    // 発射機構
    //=====
    bool catchLimit = !digitalRead(Ls_catch);
    //接触＝ture 未接触=false

    // 状態機械
    switch (nowShootState)
    {
    //=====
    // SAFE
    //=====
    case Shoot_SAFE:

        // モーター停止
        ShootMotor.stop();

        // ソレノイドPULL
        handSol.PULL();

        // Yボタンが押されたら動作開始
        if (buttonY_pressed)
        {
            changeShootState(Shoot_Moving);
        }

        break;


    //=====
    // Moving
    //=====
    case Shoot_Moving:

        // 間欠ギアを正回転
        ShootMotor.drive(-255);
        handSol.PULL();

        // 補充位置まで来たら停止
        if (catchLimit)
        {
            ShootMotor.stop();
            //状態遷移
            changeShootState(Shoot_STOP_PUSH);
        }


        // 安全モード
        if (buttonY_pressed)
        {
            changeShootState(Shoot_SAFE);
        }

        break;


    //=====
    // STOP_PUSH
    //=====
    case Shoot_STOP_PUSH:

        // モーター停止
        ShootMotor.stop();

        // ソレノイドPUSH
        handSol.PUSH();

        // PUSHが完了したら発射待ち
        if (millis() - stateStartTime >= 50)
        {
            changeShootState(Shoot_SHOOT_WAIT);
        }


        break;


    //=====
    // SHOOT_WAIT
    //=====
    case Shoot_SHOOT_WAIT:

        // モーター停止
        ShootMotor.stop();

        // L1が押されたら発射
        if (buttonL1)
        {
            changeShootState(Shoot_PUSH_LONG);
        }

        break;


    //=====
    // PUSH_LONG
    //=====
    case Shoot_PUSH_LONG:

        // モーター正回転
        ShootMotor.drive(-255);

        // ソレノイドPUSH-long
        handSol.PUSH_long();

        // 発射動作が終わった
        if (millis() - stateStartTime >= 8000)
        {
            changeShootState(Shoot_Moving);
        }

        // 安全モード
        if (buttonY_pressed)
        {
            changeShootState(Shoot_SAFE);
        }

        break;

    }
    
    //=====
    // 周期
    //=====

    delay(5);
}