#ifndef ROLLER_H
#define ROLLER_H

#include <Arduino.h>
#include <SCServo.h>

class Roller
{
public:
    // コンストラクタ（サーボインスタンス、サーボID、動作速度）
    Roller(SMS_STS& s, uint8_t id, int speed);

    // 初期化
    void begin();

    // Piから受け取ったボタン状態を設定
    void setButton(uint8_t go, uint8_t back);

    // 毎loop()呼び出す
    void update();

    // 個別のアクション
    void forward();
    void backward();
    void stop();

private:
    SMS_STS& servo;      // サーボ
    uint8_t id;          // サーボID
    int targetSpeed;     // 動作速度

    // Piから受信したボタン状態
    uint8_t buttonGo;
    uint8_t buttonBack;
};

#endif