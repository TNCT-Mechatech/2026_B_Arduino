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
      armMoving(false)
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

                armOpen = false;
                armMoving = false;
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
    return armMoving;
}