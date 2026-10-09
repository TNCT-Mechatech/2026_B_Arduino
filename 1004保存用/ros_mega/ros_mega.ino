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
//ローラ機構動作確認済み
//0930
//シリアルブリッジ再調整受信のみ

//============================================================

#include <Arduino.h>

#include "Reload.h"
#include "Motor.h"
#include "Collect.h"
#include "Roller.h"
#include "Cam02.h"
#include "LookUp.h"


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
  uint8_t buttonCollectBack;

  uint8_t buttonCamUP;     //装填カム上昇
  uint8_t buttonCamDOWN;   //装填カム戻す
  uint8_t buttonRollGo;    //ローラ回転
  uint8_t buttonRollBack;  //ローラ逆回転

  uint8_t buttonLookUp;    //発射角度上昇
  uint8_t buttonLookDown;  //発射角度下降_初期位置

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

// Mega起動確認LED緑setup中
const uint8_t LED_STARTUP = A7;

// Raspberry Piからの通信確認LED
const uint8_t LED_RX = A6;


//============================================================
// 通信LED点灯時間
//============================================================

const unsigned long LED_RX_TIME = 50;

unsigned long led_rx_on_time = 0;

//============================================================
// コントローラー入力
// Mega側の変数
// controller
// button[0] = A = 補充
// button[1] = B = 回収上昇
// button[2] = X = 回収下降拾う
// button[5] = RB(R1) = 回収回転
//
// cont2
// button[3] = Y = 装填カム上昇
// button[4] = LB(L1) = ローラ回転、雑巾を送り出すroller
// buttin[5] = RB(R1) = ローラ逆回転バックBack
//============================================================

uint8_t buttonA = 0;
uint8_t buttonB = 0;
uint8_t buttonX = 0;
uint8_t buttonCollectROT = 0;
uint8_t buttonCollectHand = 0;
uint8_t buttonCollectBack = 0;

uint8_t buttonCamUP = 0;
uint8_t buttonCamDOWN = 0;
uint8_t buttonRollGo = 0;
uint8_t buttonRollBack = 0;

uint8_t buttonLookUp = 0;
uint8_t buttonLookDown = 0;

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
  A2  // limit_7
);

//=====
// ローラ機構sts3215
//=====
SMS_STS rollerServo;

Roller roller(rollerServo, 27, -4000);

//=====
//補充装填機構
//=====

//arm
Reload arm(
  5,  // motor_1 PWM
  4,  // motor_1 DIR
  14,  // limit open LS_1
  15   // limit close LS_2
);

Cam cam(25);

//============================================================
// 補充装填機構の状態 Charge_Reload カムCam
//============================================================
enum CR_State
{
  CR_START,        // 待機
  CR_CAM_RELOAD,   // カムを補充位置へ戻す
  CR_ARM_OPEN,     // アームを開く
  CR_ARM_WAIT,       // 補充動作、開くで待機
  CR_ARM_CLOSE     // アームを閉じる
};

CR_State nowCRState = CR_START;

bool prevButtonA = false;
bool buttonAPressed = false;

bool isButtonAPressed()
{
  bool current = (buttonA != 0);

  bool pressed = current && !prevButtonA;

  prevButtonA = current;

  return pressed;
}

//補充装填機構の関数
void changeCRState(CR_State newState) {
  nowCRState = newState;
}

void updateChargeReload()
{
  switch (nowCRState)
  {
    //========================================================
    // アーム閉・待機
    //========================================================

    case CR_START:

      // Aボタンを1回押した
      if (buttonAPressed)
      {
        //====================================================
        // Aを押したら、カムの位置に関係なく
        // 必ず補充位置へ戻す
        //====================================================

        changeCRState(CR_CAM_RELOAD);
      }

      break;


    //========================================================
    // カムを補充位置へ戻す
    //========================================================

    case CR_CAM_RELOAD:

      //====================================================
      // カムが補充位置に到達した
      //====================================================

      if (cam.isReloadPosition())
      {
        changeCRState(CR_ARM_OPEN);
      }

      //====================================================
      // カムが動いていないなら
      // 補充位置への移動を開始
      //====================================================

      else if (!cam.isMoving())
      {
        cam.reload();
      }

      break;


    //========================================================
    // アームを開く
    //========================================================

    case CR_ARM_OPEN:

      //====================================================
      // 開側リミットに到達
      //====================================================

      if (arm.isOpen())
      {
        // アーム停止後、Aボタン待ち
        changeCRState(CR_ARM_WAIT);
      }

      //====================================================
      // まだ動いていなければ開く
      //====================================================

      else if (!arm.isMoving())
      {
        arm.start();
      }

      break;


    //========================================================
    // アーム開状態で待機
    //========================================================

    case CR_ARM_WAIT:

      //====================================================
      // Aボタンをもう一度押した
      //====================================================

      if (buttonAPressed)
      {
        changeCRState(CR_ARM_CLOSE);
      }

      break;


    //========================================================
    // アームを閉じる
    //========================================================

    case CR_ARM_CLOSE:

      //====================================================
      // 閉じる動作が完了
      //
      // Reload::update()内部で
      // 閉側リミット
      // ↓
      // 負荷抜き
      // ↓
      // armMoving = false
      //
      // となる
      //====================================================

      if (!arm.isOpen() && !arm.isMoving())
      {
        // 補充シーケンス完了
        changeCRState(CR_START);
      }

      //====================================================
      // まだ動いていないなら閉じる
      //====================================================

      else if (!arm.isMoving())
      {
        arm.start();
      }

      break;
  }
}

//射出角度調整
LookUp lookUp(23);

//============================================================
// setup
//============================================================

void setup() {
  //========================================================
  // LED初期化
  //========================================================

  pinMode(LED_STARTUP, OUTPUT);
  pinMode(LED_RX, OUTPUT);

  digitalWrite(LED_STARTUP, HIGH);
  digitalWrite(LED_RX, LOW);


  //========================================================
  // Raspberry Piとのシリアル通信
  //========================================================

  Serial.begin(115200);

  //フィーテックサーボ用
  Serial1.begin(1000000);
  rollerServo.pSerial = &Serial1;

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
  joy_msg.data.buttonCollectBack = 0;

  joy_msg.data.buttonCamUP = 0;
  joy_msg.data.buttonCamDOWN = 0;
  joy_msg.data.buttonRollGo = 0;
  joy_msg.data.buttonRollBack = 0;


  joy_msg.data.buttonLookUp = 0;
  joy_msg.data.buttonLookDown = 0;

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

  //射出角度調整
  lookUp.begin();

  //========================================================
  // Mega起動確認
  // A7 LED
  //========================================================

  digitalWrite(LED_STARTUP, LOW);

}


//============================================================
// loop
//============================================================

void loop() {
  //========================================================
  // Raspberry Piから受信
  //========================================================

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
    buttonCollectBack = joy_msg.data.buttonCollectBack;

    buttonCamUP = joy_msg.data.buttonCamUP;
    buttonCamDOWN = joy_msg.data.buttonCamDOWN;
    buttonRollGo = joy_msg.data.buttonRollGo;
    buttonRollBack = joy_msg.data.buttonRollBack;

    buttonLookUp = joy_msg.data.buttonLookUp;
    buttonLookDown = joy_msg.data.buttonLookDown;


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
  //=====
  //入力更新
  buttonAPressed = isButtonAPressed();

  arm.update();
  cam.update();

  updateChargeReload();


  //========================================================
  // 手動カム操作
  //========================================================

  // 補充シーケンス中は手動操作しない
  if (nowCRState == CR_START)
  {
    if (buttonCamUP)
    {
      cam.up();
    }
    else if (buttonCamDOWN)
    {
      cam.down();
    }
  } 

  //========================================================
  // 回収機構
  //========================================================
  //上昇、下降、回転、開閉
  rack.setCollectUp(buttonB);
  rack.setCollectDown(buttonX);
  rack.setCollectRotate(buttonCollectROT);
  rack.setHandButton(buttonCollectHand);
  rack.setCollectBack(buttonCollectBack);

  rack.update();

  //=====
  // ローラ機構
  //=====
  // ボタン入力を監視し、ローラを制御
  roller.setButton(buttonRollGo, buttonRollBack);
  roller.update();

  //=====
  // 射出角度調整
  //=====
  lookUp.update();

  if (buttonLookUp)
  {
    lookUp.up();
  }

  if (buttonLookDown)
  {
    lookUp.down();
  }

}