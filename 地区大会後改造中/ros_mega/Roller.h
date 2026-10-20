#ifndef ROLLER_H
#define ROLLER_H

#include <Arduino.h>
#include <SCServo.h>

class Roller
{
public:
    Roller(SMS_STS& s, uint8_t id, int speed);

    void begin();

    void setButton(uint8_t go, uint8_t back);

    void update();

    bool forward();
    bool backward();
    bool stop();

private:
    SMS_STS& servo;
    uint8_t id;
    int targetSpeed;

    uint8_t buttonGo;
    uint8_t buttonBack;

    bool communicationError;
    unsigned long errorTime;

    static const unsigned long RETRY_INTERVAL = 500;
};

#endif