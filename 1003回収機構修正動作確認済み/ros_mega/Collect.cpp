#include "Collect.h"


// ==================================================
// コンストラクタ
// ==================================================

Collect::Collect(
  uint8_t motor_3PwmPin,
  uint8_t motor_3DirPin,

  uint8_t motor_4PwmPin,
  uint8_t motor_4DirPin,

  uint8_t motor_5PwmPin,
  uint8_t motor_5DirPin,

  uint8_t limit_3,
  uint8_t limit_4,
  uint8_t limit_5,
  uint8_t limit_6,
  uint8_t limit_7
)
  : motor_3(motor_3PwmPin, motor_3DirPin),
    motor_4(motor_4PwmPin, motor_4DirPin),
    motor_5(motor_5PwmPin, motor_5DirPin),

    // STS3215
    hand(21),

    // リミットスイッチ
    limit_3Pin(limit_3),
    limit_4Pin(limit_4),
    limit_5Pin(limit_5),
    limit_6Pin(limit_6),
    limit_7Pin(limit_7),

    // 入力
    collectUp(false),
    collectDown(false),
    collectRotate(false),
    collectBack(false),
    handButton(false),

    // 前回状態
    lastCollectRotate(false),
    lastHandButton(false),

    // ハンド
    handState(OPEN),

    // M3/M4 ロック
    motor_3upLocked(false),
    motor_4upLocked(false),

    motor_3downLocked(false),
    motor_4downLocked(false),

    // M5
    rotateState(ROTATE_STOP),

    // M3 退避
    motor_3RecoveryState(RECOVERY_NONE),
    motor_3RecoveryStartTime(0),

    // M4 退避
    motor_4RecoveryState(RECOVERY_NONE),
    motor_4RecoveryStartTime(0)
{
}


// ==================================================
// begin
// ==================================================

void Collect::begin()
{
  // --------------------------------------------------
  // モーター初期化
  // --------------------------------------------------

  motor_3.begin();
  motor_4.begin();
  motor_5.begin();


  // --------------------------------------------------
  // STS3215
  // --------------------------------------------------

  Serial1.begin(1000000);
  hand.begin();


  // --------------------------------------------------
  // リミットスイッチ
  // INPUT_PULLUP
  //
  // OFF = HIGH
  // ON  = LOW
  // --------------------------------------------------

  pinMode(limit_3Pin, INPUT_PULLUP);
  pinMode(limit_4Pin, INPUT_PULLUP);
  pinMode(limit_5Pin, INPUT_PULLUP);
  pinMode(limit_6Pin, INPUT_PULLUP);
  pinMode(limit_7Pin, INPUT_PULLUP);


  // --------------------------------------------------
  // モーター停止
  // --------------------------------------------------

  motor_3.stop();
  motor_4.stop();
  motor_5.stop();


  // --------------------------------------------------
  // ロック解除
  // --------------------------------------------------

  motor_3upLocked = false;
  motor_4upLocked = false;

  motor_3downLocked = false;
  motor_4downLocked = false;


  // --------------------------------------------------
  // 退避状態リセット
  // --------------------------------------------------

  motor_3RecoveryState = RECOVERY_NONE;
  motor_3RecoveryStartTime = 0;

  motor_4RecoveryState = RECOVERY_NONE;
  motor_4RecoveryStartTime = 0;


  // --------------------------------------------------
  // 回転
  // --------------------------------------------------

  rotateState = ROTATE_STOP;
  lastCollectRotate = false;


  // --------------------------------------------------
  // ハンド
  // --------------------------------------------------

  handState = OPEN;
  lastHandButton = false;
}


// ==================================================
// 入力設定
// ==================================================

void Collect::setCollectUp(bool pressed)
{
  collectUp = pressed;
}


void Collect::setCollectDown(bool pressed)
{
  collectDown = pressed;
}


void Collect::setCollectRotate(bool pressed)
{
  collectRotate = pressed;
}


void Collect::setCollectBack(bool pressed)
{
  collectBack = pressed;
}


void Collect::setHandButton(bool pressed)
{
  handButton = pressed;
}


// ==================================================
// update
// ==================================================

void Collect::update()
{
  // ==================================================
  // ハンド
  // ==================================================

  updateHand();


  // ==================================================
  // M3
  //
  // 上限 = Ls_4
  // 下限 = Ls_3
  // ==================================================

  if (motor_3RecoveryState == RECOVERY_NONE)
  {
    // ------------------------------------------------
    // 上昇
    // ------------------------------------------------

    if (collectUp)
    {
      if (!motor_3upLocked)
      {
        // M3上限 = Ls_4
        if (digitalRead(limit_4Pin) == LOW)
        {
          motor_3.stop();

          motor_3upLocked = true;

          startMotor3RecoveryFromUpper();
        }
        else
        {
          // 上昇
          motor_3.drive(-MOTOR_3_SPEED);
        }
      }
      else
      {
        motor_3.stop();
      }
    }


    // ------------------------------------------------
    // 下降
    // ------------------------------------------------

    else if (collectDown)
    {
      if (!motor_3downLocked)
      {
        // M3下限 = Ls_3
        if (digitalRead(limit_3Pin) == LOW)
        {
          motor_3.stop();

          motor_3downLocked = true;

          startMotor3RecoveryFromLower();
        }
        else
        {
          // 下降
          motor_3.drive(MOTOR_3_SPEED);
        }
      }
      else
      {
        motor_3.stop();
      }
    }


    // ------------------------------------------------
    // 操作なし
    // ------------------------------------------------

    else
    {
      motor_3.stop();
    }
  }


  // ==================================================
  // M4
  //
  // 上限 = Ls_5
  // 下限 = Ls_6
  // ==================================================

  if (motor_4RecoveryState == RECOVERY_NONE)
  {
    // ------------------------------------------------
    // 上昇
    // ------------------------------------------------

    if (collectUp)
    {
      if (!motor_4upLocked)
      {
        // M4上限 = Ls_5
        if (digitalRead(limit_5Pin) == LOW)
        {
          motor_4.stop();

          motor_4upLocked = true;

          startMotor4RecoveryFromUpper();
        }
        else
        {
          // 上昇
          motor_4.drive(-MOTOR_4_SPEED);
        }
      }
      else
      {
        motor_4.stop();
      }
    }


    // ------------------------------------------------
    // 下降
    // ------------------------------------------------

    else if (collectDown)
    {
      if (!motor_4downLocked)
      {
        // M4下限 = Ls_6
        if (digitalRead(limit_6Pin) == LOW)
        {
          motor_4.stop();

          motor_4downLocked = true;

          startMotor4RecoveryFromLower();
        }
        else
        {
          // 下降
          motor_4.drive(MOTOR_4_SPEED);
        }
      }
      else
      {
        motor_4.stop();
      }
    }


    // ------------------------------------------------
    // 操作なし
    // ------------------------------------------------

    else
    {
      motor_4.stop();
    }
  }


  // ==================================================
  // M3/M4 退避処理
  //
  // M3とM4は完全に独立
  // ==================================================

  updateMotor3Recovery();
  updateMotor4Recovery();


  // ==================================================
  // M5 回転
  // ==================================================

  updateRotate();


  // ==================================================
  // 反対方向を押したらロック解除
  // ==================================================

  // --------------------------------------------------
  // 下降操作
  // → 上限ロック解除
  // --------------------------------------------------

  if (collectDown)
  {
    motor_3upLocked = false;
    motor_4upLocked = false;
  }


  // --------------------------------------------------
  // 上昇操作
  // → 下限ロック解除
  // --------------------------------------------------

  if (collectUp)
  {
    motor_3downLocked = false;
    motor_4downLocked = false;
  }
}


// ==================================================
// M3 上限退避開始
// ==================================================

void Collect::startMotor3RecoveryFromUpper()
{
  motor_3RecoveryState = RECOVERY_FROM_UPPER;
  motor_3RecoveryStartTime = millis();
}


// ==================================================
// M3 下限退避開始
// ==================================================

void Collect::startMotor3RecoveryFromLower()
{
  motor_3RecoveryState = RECOVERY_FROM_LOWER;
  motor_3RecoveryStartTime = millis();
}


// ==================================================
// M3 退避更新
// ==================================================

void Collect::updateMotor3Recovery()
{
  if (motor_3RecoveryState == RECOVERY_NONE)
  {
    return;
  }


  unsigned long elapsed =
    millis() - motor_3RecoveryStartTime;


  // ==================================================
  // 上限から退避
  //
  // 上限 → 少し下降
  // ==================================================

  if (motor_3RecoveryState == RECOVERY_FROM_UPPER)
  {
    // 下降
    motor_3.drive(MOTOR_3_SPEED);


    if (elapsed >= RECOVERY_TIME)
    {
      motor_3.stop();

      motor_3RecoveryState = RECOVERY_NONE;

      // 上限側をロック
      motor_3upLocked = true;

      // 反対方向は解除
      motor_3downLocked = false;
    }

    return;
  }


  // ==================================================
  // 下限から退避
  //
  // 下限 → 少し上昇
  // ==================================================

  if (motor_3RecoveryState == RECOVERY_FROM_LOWER)
  {
    // 上昇
    motor_3.drive(-MOTOR_3_SPEED);


    if (elapsed >= RECOVERY_TIME)
    {
      motor_3.stop();

      motor_3RecoveryState = RECOVERY_NONE;

      // 下限側をロック
      motor_3downLocked = true;

      // 反対方向は解除
      motor_3upLocked = false;
    }

    return;
  }
}


// ==================================================
// M4 上限退避開始
// ==================================================

void Collect::startMotor4RecoveryFromUpper()
{
  motor_4RecoveryState = RECOVERY_FROM_UPPER;
  motor_4RecoveryStartTime = millis();
}


// ==================================================
// M4 下限退避開始
// ==================================================

void Collect::startMotor4RecoveryFromLower()
{
  motor_4RecoveryState = RECOVERY_FROM_LOWER;
  motor_4RecoveryStartTime = millis();
}


// ==================================================
// M4 退避更新
// ==================================================

void Collect::updateMotor4Recovery()
{
  if (motor_4RecoveryState == RECOVERY_NONE)
  {
    return;
  }


  unsigned long elapsed =
    millis() - motor_4RecoveryStartTime;


  // ==================================================
  // 上限から退避
  //
  // 上限 → 少し下降
  // ==================================================

  if (motor_4RecoveryState == RECOVERY_FROM_UPPER)
  {
    // 下降
    motor_4.drive(MOTOR_4_SPEED);


    if (elapsed >= RECOVERY_TIME)
    {
      motor_4.stop();

      motor_4RecoveryState = RECOVERY_NONE;

      // 上限側をロック
      motor_4upLocked = true;

      // 反対方向は解除
      motor_4downLocked = false;
    }

    return;
  }


  // ==================================================
  // 下限から退避
  //
  // 下限 → 少し上昇
  // ==================================================

  if (motor_4RecoveryState == RECOVERY_FROM_LOWER)
  {
    // 上昇
    motor_4.drive(-MOTOR_4_SPEED);


    if (elapsed >= RECOVERY_TIME)
    {
      motor_4.stop();

      motor_4RecoveryState = RECOVERY_NONE;

      // 下限側をロック
      motor_4downLocked = true;

      // 反対方向は解除
      motor_4upLocked = false;
    }

    return;
  }
}


// ==================================================
// M5 回転処理
// ==================================================

void Collect::updateRotate()
{
  // ==================================================
  // R1単押し検出
  //
  // false → true になった瞬間だけ
  // 自動回転を開始
  // ==================================================

  if (collectRotate && !lastCollectRotate)
  {
    rotateState = ROTATE_AUTO;
  }


  // ==================================================
  // R1の現在状態を保存
  // ==================================================

  lastCollectRotate = collectRotate;


  // ==================================================
  // R1とL1が同時に押されている場合
  //
  // R1を優先
  //
  // ただしLs_7が押されていたら停止
  // ==================================================

  if (collectRotate)
  {
    // Ls_7
    if (digitalRead(limit_7Pin) == LOW)
    {
      motor_5.stop();

      rotateState = ROTATE_STOP;

      return;
    }


    // R1が押されている間は正回転
    motor_5.drive(-MOTOR_5_SPEED);

    return;
  }


  // ==================================================
  // L1
  //
  // R1が押されていない場合のみ
  // 逆回転
  // ==================================================

  if (collectBack)
  {
    // L1を押している間だけ逆回転
    motor_5.drive(MOTOR_5_SPEED);

    // L1を離したら自動回転には戻らない
    rotateState = ROTATE_STOP;

    return;
  }


  // ==================================================
  // 自動回転
  //
  // R1を離しても
  // ROTATE_AUTOならLs_7まで回転
  // ==================================================

  if (rotateState == ROTATE_AUTO)
  {
    // Ls_7到達
    if (digitalRead(limit_7Pin) == LOW)
    {
      motor_5.stop();

      rotateState = ROTATE_STOP;

      return;
    }


    // Ls_7まで正回転
    motor_5.drive(-MOTOR_5_SPEED);

    return;
  }


  // ==================================================
  // どの条件にも当てはまらない
  // ==================================================

  motor_5.stop();
}


// ==================================================
// ハンド
// ==================================================

void Collect::updateHand()
{
  // ==================================================
  // Sボタンの立ち上がり検出
  //
  // false → true
  // ==================================================

  if (handButton && !lastHandButton)
  {
    // ------------------------------------------------
    // OPEN → CLOSE
    // ------------------------------------------------

    if (handState == OPEN)
    {
      hand.close();
      handState = CLOSE;
    }


    // ------------------------------------------------
    // CLOSE → OPEN
    // ------------------------------------------------

    else
    {
      hand.open();
      handState = OPEN;
    }
  }


  // 前回状態を保存
  lastHandButton = handButton;
}


// ==================================================
// ハンド状態取得
// ==================================================

Collect::HandState Collect::getHandState()
{
  return handState;
}


// ==================================================
// ラック停止
// ==================================================

void Collect::stopRack()
{
  motor_3.stop();
  motor_4.stop();
}


// ==================================================
// 回転停止
// ==================================================

void Collect::stopRotate()
{
  motor_5.stop();
  rotateState = ROTATE_STOP;
}