#include "Roller.h"

// コンストラクタ
Roller::Roller(SMS_STS& s, uint8_t id, int speed)
    : servo(s),
      id(id),
      targetSpeed(speed),
      buttonGo(0),
      buttonBack(0)
{
}

// 初期設定
void Roller::begin()
{
    // サーボのトルクを有効化
    servo.EnableTorque(id, 1);

    // EPROMのロック解除
    servo.unLockEprom(id);

    // Work Mode = 1
    // 速度制御モード
    servo.writeByte(id, 33, 1);

    // EPROMを再ロック
    servo.LockEprom(id);

    // 初期状態は停止
    stop();
}


//============================================================
// Piから受信したボタン状態を設定
//============================================================

void Roller::setButton(uint8_t go, uint8_t back)
{
    buttonGo = go;
    buttonBack = back;
}


//============================================================
// 毎loop()呼び出す
//============================================================

void Roller::update()
{
    // 両方押されている場合は安全のため停止
    if (buttonGo && !buttonBack) {
        forward();
    }
    else if (buttonBack && !buttonGo) {
        backward();
    }
    else {
        stop();
    }
}


//============================================================
// 正転
//============================================================

void Roller::forward()
{
    servo.WriteSpe(id, targetSpeed);
}


//============================================================
// 逆転
//============================================================

void Roller::backward()
{
    servo.WriteSpe(id, -targetSpeed);
}


//============================================================
// 停止
//============================================================

void Roller::stop()
{
    servo.WriteSpe(id, 0);
}