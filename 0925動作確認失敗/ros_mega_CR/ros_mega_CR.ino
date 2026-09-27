//============================================================
// ros_mega.ino
// 26robocon-B
// 補充装填機構、回収機構、射出ローラ機構
// 2026-09-24
// kanayama syunya
//
// (受信)Pi >> Mega
// controller
// button[0] = A = 補充
// button[1] = B = 回収上昇
// button[2] = X = 回収下降拾う
// button[5] = RB(R1) = 回収回転
//
// cont2
// button[11] = 十字キー上 = 装填カム上昇
// button[12] = 十字キー下 = 装填カム戻す
// button[4] = LB(L1) = ローラ回転、雑巾を送り出すroller
// buttin[5] = RB(R1) = ローラ逆回転バックBack
//
// Leonardo >> Mega
// LsShootUP = 射出機構＿角度変更の上のリミットスイッチ
// LsShootDOWN = 射出機構＿角度変更の下のリミットスイッチ
//
// (返信)Mega >> Pi
// A / B / X ボタンを返信
//
// LED
// A7 : Mega起動確認
// A6 : Raspberry Piからの通信受信確認
//
//ピン配置修正確認済み
//
//0924
//シリアルブリッジ調整済み
//0925
//mega_CR.ino
//補充装填機構　動作確認用
//loop()のローラ機構コメントアウト
//============================================================

#include <Arduino.h>

#include "Reload.h"
#include "Motor.h"
#include "Collect.h"
#include "Roller.h"
#include "Cam02.h"


#include <math.h>

#include <SerialBridge.hpp>
#include <InoHardwareSerial.hpp>


//============================================================
// Raspberry Pi → Mega
//============================================================

typedef struct
{
  uint8_t buttonA;  //補充

  uint8_t buttonB;  //回収機構上昇
  uint8_t buttonX;  //回収機構下降

  uint8_t buttonCollectROT;   //回収機構回転
  uint8_t buttonCollectHand;  //回収機構ハンド開閉

  uint8_t buttonCamUP;     //装填カム上昇
  uint8_t buttonCamDOWN;   //装填カム戻す
  uint8_t buttonRollGo;    //ローラ回転
  uint8_t buttonRollBack;  //ローラ逆回転

  uint8_t limitShootUP;
  uint8_t limitShootDOWN;

} MEGA_t;


//============================================================
// Mega → Raspberry Pi
//============================================================

typedef struct
{
  uint8_t buttonA;
  uint8_t buttonB;
  uint8_t buttonX;

} MEGA_Response_t;


//============================================================
// SerialBridge
//============================================================

sb::Message<MEGA_t> joy_msg;

sb::Message<MEGA_Response_t> response_msg;

InoHardwareSerial serial(&Serial);

SerialBridge bridge(&serial);


//============================================================
// LED
//============================================================

// Mega起動確認LED
const uint8_t LED_STARTUP = A7;

// Raspberry Piからの通信確認LED
const uint8_t LED_RX = A6;


//============================================================
// 通信LED点灯時間
//============================================================

const unsigned long LED_RX_TIME = 50;

unsigned long led_rx_on_time = 0;


//============================================================
// ピン配置
//============================================================


//============================================================
// 回収機構
//============================================================
Collect rack(
  9,  // motor_3 PWM  回収下
  8,  // motor_3 DIR

  11,  // motor_4 PWM
  10,  // motor_4 DIR

  7,  // motor_5 PWM
  6,  // motor_5 DIR

  16,  // limit_3
  17,  // limit_4
  20,  // limit_5
  21,  // limit_6
  A2,  // limit_7
  A1   // limit_8
);


//=====
// ローラ機構sts3215
//=====
SMS_STS rollerServo;

Roller roller(rollerServo, 27, 1000);

//=====
//補充装填機構
//=====

//arm
Reload arm(
  13,  // motor_1 PWM
  12,  // motor_1 DIR
  14,  // limit open LS_1
  15   // limit close LS_2
);

//装填カム
Cam cam(25);
//============================================================
// 補充装填機構の状態 Charge_Reload カムCam
//============================================================
enum CR_State
{
    CR_START,           // 通常状態
    CR_CAM_RELOADING,   // カムを補充位置へ移動
    CR_ARM_MOVING       // 補充動作中
};

CR_State nowCRState = CR_START;

//Aボタン立ち上がり検知用
uint8_t prevButtonA = 0;

//補充装填機構の状態変更の関数
void changeCRState(CR_State newState) {
  nowCRState = newState;
}

//============================================================
// コントローラー入力
// Mega側の変数
// controller
// button[0] = A = 補充
// button[1] = B = 回収上昇
// button[2] = X = 回収下降拾う
// button[5] = RB(R1) = 回収回転
// button[7] = S = 回収ハンド開閉
//
// cont2
// button[11] = 上（十字キー）= 装填カムUP
// button[12] = 下（十字キー）= 装填カム戻す
// button[4] = LB(L1) = ローラ回転、雑巾を送り出すroller
// buttin[5] = RB(R1) = ローラ逆回転バックBack
//============================================================

uint8_t buttonA = 0;
uint8_t buttonB = 0;
uint8_t buttonX = 0;
uint8_t buttonCollectROT = 0;
uint8_t buttonCollectHand = 0;

uint8_t buttonCamUP = 0;
uint8_t buttonCamDOWN = 0;
uint8_t buttonRollGo = 0;
uint8_t buttonRollBack = 0;

uint8_t limitShootUP;
uint8_t limitShootDOWN;


//============================================================
// setup
//============================================================

void setup() {
  //========================================================
  // LED初期化
  //========================================================

  pinMode(LED_STARTUP, OUTPUT);
  pinMode(LED_RX, OUTPUT);

  digitalWrite(LED_STARTUP, LOW);
  digitalWrite(LED_RX, LOW);


  //========================================================
  // Raspberry Piとのシリアル通信
  //========================================================

  Serial.begin(115200);

  //フィーテックサーボ用
  Serial1.begin(1000000);


  //========================================================
  // SerialBridge
  //========================================================

  // Pi → Mega
  bridge.add_frame(
    0,
    &joy_msg);


  // Mega → Pi
  bridge.add_frame(
    1,
    &response_msg);


  //========================================================
  // 初期値
  //========================================================
  joy_msg.data.buttonA = 0;
  joy_msg.data.buttonB = 0;
  joy_msg.data.buttonX = 0;

  joy_msg.data.buttonCollectROT = 0;
  joy_msg.data.buttonCollectHand = 0;

  joy_msg.data.buttonCamUP = 0;
  joy_msg.data.buttonCamDOWN = 0;
  joy_msg.data.buttonRollGo = 0;
  joy_msg.data.buttonRollBack = 0;


  joy_msg.data.limitShootUP = 0;
  joy_msg.data.limitShootDOWN = 0;

  response_msg.data.buttonA = 0;
  response_msg.data.buttonB = 0;
  response_msg.data.buttonX = 0;

  //========================================================
  // 補充装填機構
  //========================================================
  //補充
  arm.begin();
  //装填カム
  cam.begin();

  //========================================================
  // 回収機構
  //========================================================
  rack.begin();

  //=====
  // ローラ機構
  //=====
  roller.begin();


  //========================================================
  // リミットスイッチ
  //========================================================

  //pinMode(limit_9,INPUT_PULLUP);

  //========================================================
  // Mega起動確認
  // A7 LED
  //========================================================

  digitalWrite(LED_STARTUP, HIGH);

  delay(200);

  digitalWrite(LED_STARTUP, LOW);
}


//============================================================
// loop
//============================================================

void loop() {
  // シリアルブリッジ更新
  bridge.update();

  //========================================================
  // 新しいデータを受信した場合
  //========================================================

  if (joy_msg.was_updated()) {
    //======================================================
    // Raspberry Piから通信を受信したことを表示
    // A6 LED
    //======================================================

    digitalWrite(LED_RX, HIGH);

    led_rx_on_time = millis();


    //======================================================
    // A / B / X を取得
    //======================================================

    buttonA = joy_msg.data.buttonA;
    buttonB = joy_msg.data.buttonB;
    buttonX = joy_msg.data.buttonX;

    buttonCollectROT = joy_msg.data.buttonCollectROT;
    buttonCollectHand = joy_msg.data.buttonCollectHand;

    buttonCamUP = joy_msg.data.buttonCamUP;
    buttonCamDOWN = joy_msg.data.buttonCamDOWN;
    buttonRollGo = joy_msg.data.buttonRollGo;
    buttonRollBack = joy_msg.data.buttonRollBack;


    //======================================================
    // Piへ受信したデータを返信
    //======================================================

    response_msg.data.buttonA = buttonA;
    response_msg.data.buttonB = buttonB;
    response_msg.data.buttonX = buttonX;

    bridge.write(1);
  }
  //========================================================
  // A6通信LED OFF
  //========================================================
  if (
    digitalRead(LED_RX) == HIGH && millis() - led_rx_on_time >= LED_RX_TIME) {
    digitalWrite(LED_RX, LOW);
  }


  //========================================================
  // 補充装填機構
  //CR_START,           // 通常状態
  //CR_CAM_RELOADING,   // カムを補充位置へ移動
  //CR_ARM_MOVING       // 補充動作中
  //========================================================
  cam.update();

  // Aボタン立ち上がり検出
  bool buttonA_pressed = (buttonA != 0 && prevButtonA == 0);
  prevButtonA = buttonA;

  //========================================================
  // 補充装填機構
  //========================================================

  switch (nowCRState)
  {
        //======================================================
        // 通常状態
        //======================================================

        case CR_START:
        {
            //==================================================
            // Aボタン
            //==================================================

            if (buttonA_pressed)
            {
                // ---------------------------------------------
                // 補充開始
                //
                // 現在のカム角度に関係なく、
                // まず補充位置へ戻す
                // ---------------------------------------------

                cam.setReloadLock(true);

                cam.reload();

                // ---------------------------------------------
                // すでに補充位置にいる場合
                // ---------------------------------------------

                if (cam.isAtReload())
                {
                    // カム位置OK
                    // 補充開始
                    arm.start();

                    changeCRState(CR_ARM_MOVING);
                }
                else
                {
                    // カムが補充位置へ移動中
                    changeCRState(CR_CAM_RELOADING);
                }
            }


            //==================================================
            // カム角度調整
            //
            // 補充中ではないため操作可能
            //==================================================

            if (buttonCamUP)
            {
                cam.up();
            }

            if (buttonCamDOWN)
            {
                cam.down();
            }

            break;
        }


        //======================================================
        // カムを補充位置へ戻している
        //======================================================

        case CR_CAM_RELOADING:
        {
            //==================================================
            // ここではUP/DOWNを完全に無視
            //==================================================

            // ---------------------------------------------
            // カムが動作終了した
            // ---------------------------------------------

            if (!cam.isMoving())
            {
                // ---------------------------------------------
                // 補充位置に到達したか確認
                //
                // 到達していればそのまま補充開始
                //
                // 到達していなくても、
                // 今回の機構では干渉しないため
                // 補充動作は開始する
                // ---------------------------------------------

                if (cam.isAtReload())
                {
                    // 補充位置到達
                }
                else
                {
                    // カムタイムアウト等
                    //
                    // 補充は開始する。
                    // デバッグ時にカム側の問題を
                    // 確認できるようにする。
                }

                //================================================
                // 補充開始
                //================================================

                arm.start();

                changeCRState(CR_ARM_MOVING);
            }

            break;
        }


        //======================================================
        // 補充動作中
        //======================================================

        case CR_ARM_MOVING:
        {
            //==================================================
            // 補充部更新
            //==================================================

            arm.update();


            //==================================================
            // カムを補充位置に維持
            //==================================================
            //
            // 補充中は負荷がかかるため、
            // 定期的に800位置を再指令する。
            //
            // UP/DOWNはこの状態では処理しない。
            //==================================================

            cam.holdReload();


            //==================================================
            // 補充完了
            //==================================================

            if (!arm.isMoving())
            {
                // ---------------------------------------------
                // 補充終了
                // ---------------------------------------------
                
                cam.setReloadLock(false);

                changeCRState(CR_START);
            }

            break;
        }
    }

  //========================================================
  // 回収機構
  //========================================================
  //上昇、下降、回転、開閉
  //rack.setCollectUp(buttonB);
  //rack.setCollectDown(buttonX);
  //rack.setCollectRotate(buttonCollectROT);
  //rack.setHandButton(buttonCollectHand);

  //rack.update();

  //=====
  // ローラ機構
  //=====
  // ボタン入力を監視し、ローラを制御
  //roller.setButton(buttonRollGo, buttonRollBack);
  //roller.update();

}