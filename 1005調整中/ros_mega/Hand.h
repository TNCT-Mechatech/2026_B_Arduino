#ifndef HAND_H
#define HAND_H

#include <Arduino.h>
#include <SCServo.h>

class Hand
{
public:
    Hand(uint8_t id);

    void begin();

    // ハンド動作開始
    void open();
    void close();

    // 毎loop()呼び出す
    void update();

    // 現在動作中か
    bool isMoving();

private:
    SMS_STS servo;
    uint8_t id;

    const int OPEN_POS = 3150;
    const int CLOSE_POS = 3520;

    const int HAND_SPEED = 2000;
    const int ACC = 50;

    // 目標位置との許容誤差
    const int POSITION_TOLERANCE = 20;

    // サーボ動作の最大待ち時間
    const unsigned long HAND_TIMEOUT = 1000;

    enum HandState
    {
        HAND_IDLE,
        HAND_OPENING,
        HAND_CLOSING
    };

    HandState state;

    int targetPos;

    // 動作開始時刻
    unsigned long startTime;

    // 目標位置に到達したか確認
    bool isTargetReached();
};

#endif