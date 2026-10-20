//=====
// Leonardo
//
// Raspberry Pi → Leonardo
// 足回り
// joyX     : int16_t
// joyY     : int16_t
// joyRot   : int16_t
//
// 射出
// Leonardo → Raspberry Pi
//
// buttonGear 押して間欠ギア回転
// buttonGearBack 押して間欠ギア逆転
// buttonSol    ソレノイド
//
//
// 合計
// Pi → Leonardo : 9Byte
// Leonardo → Pi : 0Byte
//
//=====
// 十字キー移動の実装確認済み
// 0926
// Gear,Sol実装
// 0930
// シリアルブリッジ再調整
// 1004
// 間欠ギア逆転実装
// 1005
// ホールセンサーの仮プログラム
// 1010
// 射出機構の状態をLEDでデバック

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

    uint8_t buttonGear;
    uint8_t buttonGearBack;
    uint8_t buttonSol;

} JoyData_t;


//=====
// Leonardo → Raspberry Pi
//=====

typedef struct
{
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
// ホールセンサー
//=====
// Hall_catch 射出機構の補充地点

const int Hall_catch = A0;

//=====
// LED
//=====
//青setup中
const int LED_STARTUP = A3; 

// Riからの通信確認
const int LED_RX = A4;
//射出の状態をデバック
const int LED_Sol = A5;

//=====
// 通信LED点灯時間
//=====

const unsigned long LED_RX_TIME = 50;

unsigned long led_rx_on_time = 0;


//=====
// Motor
//=====

Motor GearMotor(pwmPin5, dirPin5);

//=====
//TppSol
//=====
TppSol handSol(pwmPinSol, dirPinSol, 50);

//=====
// コントローラー入力
//=====

int joyX = 0;
int joyY = 0;
int joyRot = 0;

uint8_t buttonGear = 0;
uint8_t buttonGearBack = 0;

uint8_t buttonSol = 0;


//=====
// 射出機構の状態
//=====
// Gear
enum GearState
{
    Gear_Moving,
    Gear_STOP,
    Gear_ReSTART,
    Gear_Shoot
    /*
    Moving = ボタン押している間、間欠ギア（正回転）可能な状態、初期状態
    Stop = Hall_catchが反応し、一時停止した状態、ボタン押しても間欠ギア回転不可
    ReSTART = ボタンソレノイドが１回以上をおされたら、間欠ギアが再回転できるようになる状態
    Shoot = 間欠ギアを再回転させ、射出中の状態でHall_catchの入力を無視する状態
    */
};

GearState nowGearState = Gear_Moving;

//Sol
enum SolState
{
    Sol_PULL,
    Sol_PUSH
    /*
    PULL = 保持部開く、一瞬通電
    PUSH = 保持部閉じる、常に通電
    */
};

SolState nowSolState = Sol_PUSH;

//button
bool oldButtonSol = false;
bool buttonSolPressed = false;

bool solPushWaiting = false;

//発射機構の状態機械
void changeGearState(GearState newState)
{
    nowGearState = newState;
}

void updateChargeShoot()
{
    //==================================================
    // Solボタン処理
    //==================================================

    if (buttonSolPressed)
    {
    // PUSHの50ms実行中だけ無視
    if (!handSol.isPushActive())
        {
            if (nowSolState == Sol_PUSH)
            {
                // PUSH → PULL
                handSol.PULL();

                nowSolState = Sol_PULL;

                solPushWaiting = false;
            }
            else
            {
                // PULL → PUSH
                handSol.PUSH();

                nowSolState = Sol_PUSH;

                // 今回の操作によるPUSH完了を待つ
                solPushWaiting = true;
            }
        }
    }

    //==================================================
    // Gear状態
    //==================================================

    switch (nowGearState)
    {
        //================================================
        // 通常状態
        //================================================
        case Gear_Moving:
            digitalWrite(LED_Sol, HIGH);

            if (digitalRead(Hall_catch) == LOW)
            {
                GearMotor.stop();

                nowGearState = Gear_STOP;

                break;
            }
            if (digitalRead(Hall_catch) == HIGH)
            {
                if (buttonGear)
                {
                    GearMotor.drive(-255);
                }
                else if (buttonGearBack)
                {
                    GearMotor.drive(255);
                }
                else
                {
                    GearMotor.stop();
                }
            }
            

            

            break;


        //================================================
        // Hall_catchで停止
        //================================================
        case Gear_STOP:

            GearMotor.stop();

            //d
            digitalWrite(LED_Sol, LOW);

            // 今回PUSHを開始していた場合
            if (solPushWaiting)
            {
                if (handSol.consumePushCompleted())
                {
                    solPushWaiting = false;

                    nowGearState = Gear_ReSTART;
                }
            }

            // PULLにした場合は即座にReSTART
            else if (buttonSolPressed && nowSolState == Sol_PULL)
            {
                // PUSH完了待ちを解除
                solPushWaiting = false;

                nowGearState = Gear_ReSTART;
            }

            break;


        //================================================
        // 再始動待ち
        //================================================
        case Gear_ReSTART:
            digitalWrite(LED_Sol, LOW);

            if (buttonGear)
            {
                GearMotor.drive(-255);

                nowGearState = Gear_Shoot;
            }
            else if (buttonGearBack)
            {
                GearMotor.drive(255);
            }
            else
            {
                GearMotor.stop();
            }

            break;


        //================================================
        // 再始動後
        //================================================
        case Gear_Shoot:

            // Hall_catchは無視
            digitalWrite(LED_Sol, LOW);

            if (buttonGear)
            {
                GearMotor.drive(-255);
            }
            else if (buttonGearBack)
            {
                GearMotor.drive(255);
            }
            else
            {
                GearMotor.stop();
            }

            // 今回のSol操作でPUSHになった場合のみ
            if (solPushWaiting)
            {
                if (handSol.consumePushCompleted())
                {
                    solPushWaiting = false;

                    nowGearState = Gear_Moving;
                }
            }

            break;
    }
}

//=====
// setup
//=====

void setup()
{
    //=====
    // LED初期化
    //=====
    pinMode(LED_STARTUP, OUTPUT);
    pinMode(LED_RX, OUTPUT);
    pinMode(LED_Sol, OUTPUT);

    digitalWrite(LED_STARTUP, HIGH);
    digitalWrite(LED_RX, LOW);
    digitalWrite(LED_Sol, LOW);



    //=====
    // Raspberry Pi通信
    //=====

    Serial.begin(230400);


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

    GearMotor.begin();

    // TppSol
    handSol.begin();
    handSol.PUSH();


    //=====
    // ホールセンサー
    //=====
    pinMode(Hall_catch, INPUT); //L3

    //=====
    //初期状態
    


    //=====
    // 初期値
    //=====

    joy_msg.data.joyX = 0;
    joy_msg.data.joyY = 0;
    joy_msg.data.joyRot = 0;


    joy_msg.data.buttonGear = 0;
    joy_msg.data.buttonGearBack = 0;
    joy_msg.data.buttonSol = 0;


    //=====
    // 起動確認
    //=====

    digitalWrite(LED_STARTUP, LOW);
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
        //Piから通信を受信したらLEDを点灯
        digitalWrite(LED_RX, HIGH);

        led_rx_on_time = millis();

        //=====
        // コントローラー入力
        //=====

        joyX =
            joy_msg.data.joyX;

        joyY =
            joy_msg.data.joyY;

        joyRot =
            joy_msg.data.joyRot;

        buttonGear =
            joy_msg.data.buttonGear;
        
        buttonGearBack = 
            joy_msg.data.buttonGearBack;

        buttonSol =
            joy_msg.data.buttonSol;



        //=====
        // Raspberry Piへ返信
        //=====
    

        bridge.write(1);
    }

    //========================================================
    // A6通信LED OFF
    //========================================================

    if (digitalRead(LED_RX) == HIGH && millis() - led_rx_on_time >= LED_RX_TIME)
    {
        digitalWrite(LED_RX, LOW);
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

    //=====
    // 発射機構
    //=====
    // SolのPUSH時間管理
    //==================================================
    handSol.update();


    //==================================================
    // Solボタンの立ち上がり検出
    //==================================================
    buttonSolPressed =
        buttonSol && !oldButtonSol;

    oldButtonSol = buttonSol;


    //==================================================
    // Gear + Sol
    //==================================================
    updateChargeShoot();

    //=====
    // 周期
    //=====

    //delay(1);
}
