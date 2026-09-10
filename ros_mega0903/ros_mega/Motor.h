#pragma once  //ヘッダーファイルを１回だけ読み込む＋事故防止になる

#include <Arduino.h>

//クラス宣言
class Motor
{
  //外部から使える機能
  public:
    Motor(uint8_t pwmPin, uint8_t dirPin);

    void begin();

    void drive(int speed); //speed = -255 ~ 255

    void stop();

//モータ内部が持っている情報
private:
  uint8_t pwmPin;
  uint8_t dirPin;
};
