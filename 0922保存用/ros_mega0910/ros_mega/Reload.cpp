#include "Reload.h"

Reload::Reload(
    uint8_t motorPwmPin,
    uint8_t motorDirPin,
    uint8_t limitOpenPin,
    uint8_t limitClosePin
)
    : motor(motorPwmPin, motorDirPin),
      limitOpenPin(limitOpenPin),
      limitClosePin(limitClosePin),
      armOpen(false),
      armMoving(false),
      releasing(false),
      releaseStartTime(0)
{
}

void Reload::begin()
{
    motor.begin();

    pinMode(limitOpenPin, INPUT_PULLUP);
    pinMode(limitClosePin, INPUT_PULLUP);

    motor.stop();
}

void Reload::update()
{
    // 閉じた後の負荷抜き動作
    if(releasing == true)
    {
        //開く
        motor.drive(MOTOR_SPEED);

        if(millis() - releaseStartTime >= RELEASE_TIME)
        {
            motor.stop();

            releasing = false;
            armMoving = false;

            //閉じている
            armOpen = false;
        }

        return;
    }


    // 動作中
    if (armMoving == true)
    {
        // 閉じている → 開く
        if (armOpen == false)
        {
            motor.drive(MOTOR_SPEED);

            if (digitalRead(limitOpenPin) == LOW)
            {
                motor.stop();

                armOpen = true;
                armMoving = false;
            }
        }

        // 開いている → 閉じる
        else
        {
            motor.drive(-MOTOR_SPEED);

            if (digitalRead(limitClosePin) == LOW)
            {
                motor.stop();

                //閉じたので負荷抜き動作へ
                releasing = true;
                releaseStartTime = millis();
            }
        }
    }

    // 動作していない
    else
    {
        motor.stop();
    }
}

void Reload::start()
{
    if (armMoving == false)
    {
        armMoving = true;
    }
}

void Reload::stop()
{
    armMoving = false;
    motor.stop();
}

bool Reload::isOpen()
{
    return armOpen;
}

bool Reload::isMoving()
{
    return armMoving || releasing;
}