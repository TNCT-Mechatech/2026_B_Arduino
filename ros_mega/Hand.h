#ifndef HAND_H
#define HAND_H

#include <Arduino.h>
#include <SCServo.h>

class Hand
{
public:
    Hand(uint8_t id);

    void begin();
    void open();
    void close();

private:
    SMS_STS servo;
    uint8_t id;

    const int OPEN_POS = 3080;
    const int CLOSE_POS = 2475;

    const int HAND_SPEED = 2000;
    const int ACC = 50;

    // 目標位置との許容誤差
    const int POSITION_TOLERANCE = 20;
};

#endif