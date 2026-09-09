#include "Hand.h"

Hand::Hand(uint8_t id)
{
    this->id = id;
}

void Hand::begin()
{
    servo.pSerial = &Serial1;
}

void Hand::open()
{
    servo.WritePosEx(
        id,
        OPEN_POS,
        HAND_SPEED,
        ACC
    );

    while (true)
    {
        int currentPos = servo.ReadPos(id);

        if (currentPos == -1)
        {
            delay(10);
            continue;
        }

        if (abs(currentPos - OPEN_POS) <= POSITION_TOLERANCE)
        {
            break;
        }

        delay(10);
    }
}

void Hand::close()
{
    servo.WritePosEx(
        id,
        CLOSE_POS,
        HAND_SPEED,
        ACC
    );

    while (true)
    {
        int currentPos = servo.ReadPos(id);

        if (currentPos == -1)
        {
            delay(10);
            continue;
        }

        if (abs(currentPos - CLOSE_POS) <= POSITION_TOLERANCE)
        {
            break;
        }

        delay(10);
    }
}