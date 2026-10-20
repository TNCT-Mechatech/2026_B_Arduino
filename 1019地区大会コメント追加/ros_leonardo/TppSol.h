#ifndef TPP_SOL_H
#define TPP_SOL_H

#include <Arduino.h>

class TppSol
{
public:

    enum State
    {
        STATE_PUSH,
        STATE_PULL
    };

private:

    int _pwm_pin;
    int _dir_pin;

    unsigned long _pushTime;
    unsigned long _stateStartTime;

    State _state;

    bool _pushActive;
    bool _pushCompleted;

public:

    TppSol(
        int pwm_pin,
        int dir_pin,
        unsigned long pushTime
    );

    void begin();

    void PULL();

    void PUSH();

    void update();

    State getState();

    bool isPushActive();

    bool consumePushCompleted();
};

#endif