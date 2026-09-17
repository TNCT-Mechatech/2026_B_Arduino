#ifndef RELOAD_H
#define RELOAD_H


#include <Arduino.h>
#include "Motor.h"

class Reload
{
public:
    Reload(
        uint8_t motorPwmPin,
        uint8_t motorDirPin,
        uint8_t limitOpenPin,
        uint8_t limitClosePin
    );

    void begin();
    void update();

    void start();
    void stop();

    bool isOpen();
    bool isMoving();

private:
    Motor motor;

    uint8_t limitOpenPin;
    uint8_t limitClosePin;

    bool armOpen;
    bool armMoving;

    bool releasing;
    unsigned long releaseStartTime;

    // 閉じたとき後に逆回転する時間
    static const unsigned long RELEASE_TIME = 200; //(ms)
    static const int MOTOR_SPEED = 255; //(最速に調整2026-08-31)
};

#endif