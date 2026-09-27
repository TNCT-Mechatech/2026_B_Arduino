#include "Cam02.h"


//============================================================
// コンストラクタ
//============================================================

Cam::Cam(uint8_t id)
{
    this->id = id;

    state = CAM_IDLE;

    targetPos = RELOAD_POS;

    startTime = 0;

    lastHoldTime = 0;

    reloadLock = false;
}


//============================================================
// 初期化
//============================================================

void Cam::begin()
{
    servo.pSerial = &Serial1;
}


//============================================================
// 正方向へ回転
//============================================================

void Cam::up()
{
    // 補充中は操作禁止
    if (reloadLock)
    {
        return;
    }

    // 補充位置へ戻している途中は操作禁止
    if (state == CAM_RELOADING)
    {
        return;
    }

    // 現在位置を取得
    int currentPos = servo.ReadPos(id);

    if (currentPos == -1)
    {
        return;
    }

    // 正方向へ進める
    targetPos =
        currentPos + (CAM_STEP * CAM_DIRECTION);

    // 0～4095から出ないようにする
    if (targetPos > MAX_POS)
    {
        targetPos = MAX_POS;
    }

    if (targetPos < MIN_POS)
    {
        targetPos = MIN_POS;
    }

    servo.WritePosEx(
        id,
        targetPos,
        CAM_SPEED,
        ACC
    );

    startTime = millis();

    state = CAM_MOVING;
}


//============================================================
// 逆方向へ回転
//============================================================

void Cam::down()
{
    // 補充中は操作禁止
    if (reloadLock)
    {
        return;
    }

    // 補充位置へ戻している途中は操作禁止
    if (state == CAM_RELOADING)
    {
        return;
    }

    // 現在位置を取得
    int currentPos = servo.ReadPos(id);

    if (currentPos == -1)
    {
        return;
    }

    // 逆方向へ進める
    targetPos =
        currentPos - (CAM_STEP * CAM_DIRECTION);

    // 0～4095から出ないようにする
    if (targetPos > MAX_POS)
    {
        targetPos = MAX_POS;
    }

    if (targetPos < MIN_POS)
    {
        targetPos = MIN_POS;
    }

    servo.WritePosEx(
        id,
        targetPos,
        CAM_SPEED,
        ACC
    );

    startTime = millis();

    state = CAM_MOVING;
}


//============================================================
// 補充位置へ戻す
//============================================================

void Cam::reload()
{
    // すでに補充位置へ戻している途中なら何もしない
    if (state == CAM_RELOADING)
    {
        return;
    }

    int currentPos = servo.ReadPos(id);

    if (currentPos == -1)
    {
        return;
    }

    // すでに補充位置なら何もしない
    if (abs(currentPos - RELOAD_POS) <= POSITION_TOLERANCE)
    {
        targetPos = RELOAD_POS;
        state = CAM_IDLE;

        return;
    }

    // 補充位置へ移動
    servo.WritePosEx(
        id,
        RELOAD_POS,
        CAM_SPEED,
        ACC
    );

    targetPos = RELOAD_POS;

    startTime = millis();

    state = CAM_RELOADING;
}


//============================================================
// 補充位置を維持する
//
// 補充部が動いている間は、カムに負荷がかかるため
// 定期的に補充位置を再指令する。
//
// ここでは毎loop送信せず、一定時間ごとに送信する。
//============================================================

void Cam::holdReload()
{
    // 補充位置維持中は必ず800を目標にする
    targetPos = RELOAD_POS;

    // 一定時間経過するまで再指令しない
    if (millis() - lastHoldTime < RELOAD_HOLD_INTERVAL)
    {
        return;
    }

    lastHoldTime = millis();

    // 現在位置を確認
    int currentPos = servo.ReadPos(id);

    if (currentPos == -1)
    {
        // 位置取得できなくても次回再試行
        return;
    }

    // 800付近なら再指令不要
    if (abs(currentPos - RELOAD_POS) <= POSITION_TOLERANCE)
    {
        return;
    }

    // 800へ戻す
    servo.WritePosEx(
        id,
        RELOAD_POS,
        CAM_SPEED,
        ACC
    );

    state = CAM_RELOADING;
    startTime = millis();
}


//============================================================
// 毎loop()呼び出す
//============================================================

void Cam::update()
{
    // 動作していなければ何もしない
    if (state == CAM_IDLE)
    {
        return;
    }

    // 現在位置を取得
    int currentPos = servo.ReadPos(id);

    // 現在位置が正常に取得できた場合
    if (currentPos != -1)
    {
        // 目標位置に到達
        if (abs(currentPos - targetPos) <= POSITION_TOLERANCE)
        {
            state = CAM_IDLE;
            return;
        }
    }

    // タイムアウト
    if (millis() - startTime >= CAM_TIMEOUT)
    {
        state = CAM_IDLE;
        return;
    }
}


//============================================================
// 現在動作中か
//============================================================

bool Cam::isMoving()
{
    return state != CAM_IDLE;
}


//============================================================
// 補充位置にいるか
//============================================================

bool Cam::isAtReload()
{
    int currentPos = servo.ReadPos(id);

    if (currentPos == -1)
    {
        return false;
    }

    return abs(currentPos - RELOAD_POS)
           <= POSITION_TOLERANCE;
}


//============================================================
// 補充中のカムロック
//============================================================

void Cam::setReloadLock(bool lock)
{
    reloadLock = lock;

    if (lock)
    {
        // ロック開始時は補充位置維持タイマーをリセット
        lastHoldTime = millis();

        targetPos = RELOAD_POS;
    }
}