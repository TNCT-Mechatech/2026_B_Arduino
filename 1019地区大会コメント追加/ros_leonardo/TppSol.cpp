#include "TppSol.h"

TppSol::TppSol(
    int pwm_pin,
    int dir_pin,
    unsigned long pushTime
)
{
    _pwm_pin = pwm_pin;
    _dir_pin = dir_pin;

    _pushTime = pushTime;
    _stateStartTime = 0;

    _state = STATE_PUSH;

    _pushActive = false;
    _pushCompleted = false;
}


void TppSol::begin()
{
    pinMode(_pwm_pin, OUTPUT);
    pinMode(_dir_pin, OUTPUT);

    // 初期出力OFF
    digitalWrite(_pwm_pin, LOW);

    // PUSH方向
    digitalWrite(_dir_pin, LOW);

    _state = STATE_PUSH;

    _pushActive = false;
    _pushCompleted = false;
    _stateStartTime = 0;
}


void TppSol::PULL()
{
    // D2 HIGH → PULL
    digitalWrite(_dir_pin, LOW);

    // 通電ON
    digitalWrite(_pwm_pin, HIGH);

    _state = STATE_PULL;

    _pushActive = false;
    _pushCompleted = false;
    _stateStartTime = 0;
}


void TppSol::PUSH()
{
    // D2 LOW → PUSH
    digitalWrite(_dir_pin, HIGH);

    // 通電ON
    digitalWrite(_pwm_pin, HIGH);

    _state = STATE_PUSH;

    _pushActive = true;

    _pushCompleted = false;

    _stateStartTime = millis();
}


void TppSol::update()
{
    // PUSH中でなければ何もしない
    if (!_pushActive)
    {
        return;
    }

    unsigned long currentTime = millis();

    if (currentTime - _stateStartTime >= _pushTime)
    {
        // PUSH終了
        digitalWrite(_pwm_pin, LOW);

        _pushActive = false;

        _stateStartTime = 0;

        // PUSH完了イベント
        _pushCompleted = true;
    }
}


TppSol::State TppSol::getState()
{
    return _state;
}


bool TppSol::isPushActive()
{
    return _pushActive;
}


bool TppSol::consumePushCompleted()
{
    if (_pushCompleted)
    {
        _pushCompleted = false;

        return true;
    }

    return false;
}