#include "Roller.h"

Roller::Roller(SMS_STS& s, uint8_t id, int speed)
    : servo(s),
      id(id),
      targetSpeed(speed),
      buttonGo(0),
      buttonBack(0),
      communicationError(false),
      errorTime(0)
{
}

void Roller::begin()
{
    servo.pSerial = &Serial1;

    servo.EnableTorque(id, 1);

    servo.unLockEprom(id);
    servo.writeByte(id, 33, 1);
    servo.LockEprom(id);

    stop();
}

void Roller::setButton(uint8_t go, uint8_t back)
{
    buttonGo = go;
    buttonBack = back;
}

void Roller::update()
{
    // 通信エラー中
    if (communicationError)
    {
        // 500ms経過するまで再通信しない
        if (millis() - errorTime < RETRY_INTERVAL)
        {
            return;
        }

        // 再通信を試す
        errorTime = millis();
    }

    bool result;

    if (buttonGo && !buttonBack)
    {
        result = forward();
    }
    else if (buttonBack && !buttonGo)
    {
        result = backward();
    }
    else
    {
        result = stop();
    }

    if (!result)
    {
        // 通信失敗
        communicationError = true;
        errorTime = millis();
        return;
    }

    // 通信成功
    communicationError = false;
}

bool Roller::forward()
{
    return servo.WriteSpe(id, targetSpeed) != 0;
}

bool Roller::backward()
{
    return servo.WriteSpe(id, -targetSpeed) != 0;
}

bool Roller::stop()
{
    return servo.WriteSpe(id, 0) != 0;
}