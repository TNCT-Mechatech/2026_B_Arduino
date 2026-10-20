#include "Hand.h"

Hand::Hand(uint8_t id)
{
    this->id = id;

    state = HAND_IDLE;
    targetPos = 0;
    startTime = 0;
}

void Hand::begin()
{
    servo.pSerial = &Serial1;
}

void Hand::open()
{
    // すでに開く動作中なら何もしない
    if (state == HAND_OPENING)
    {
        return;
    }

    servo.WritePosEx(id, OPEN_POS, HAND_SPEED, ACC);

    targetPos = OPEN_POS;
    startTime = millis();
    state = HAND_OPENING;
}

void Hand::close()
{
    // すでに閉じる動作中なら何もしない
    if (state == HAND_CLOSING)
    {
        return;
    }

    servo.WritePosEx(id, CLOSE_POS, HAND_SPEED, ACC);

    targetPos = CLOSE_POS;
    startTime = millis();
    state = HAND_CLOSING;
}

void Hand::update()
{
    // 動作していなければ何もしない
    if (state == HAND_IDLE)
    {
        return;
    }

    // 現在位置を取得
    int currentPos = servo.ReadPos(id);

    // 現在位置が正常に取得できた場合
    if (currentPos != -1)
    {
        // 目標位置に到達した
        if (abs(currentPos - targetPos) <= POSITION_TOLERANCE)
        {
            state = HAND_IDLE;
            return;
        }
    }

    // タイムアウト
    if (millis() - startTime >= HAND_TIMEOUT)
    {
        state = HAND_IDLE;
        return;
    }
}

bool Hand::isMoving()
{
    return state != HAND_IDLE;
}