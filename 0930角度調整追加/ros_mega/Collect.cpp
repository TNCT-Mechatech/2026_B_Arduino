#include "Collect.h"

// ============================================================
// コンストラクタ
// ============================================================

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
  uint8_t limit_7,
  uint8_t limit_8
)
  : motor_3(motor_3PwmPin, motor_3DirPin),
    motor_4(motor_4PwmPin, motor_4DirPin),
    motor_5(motor_5PwmPin, motor_5DirPin),

    hand(21),   // STS3215 ID = 21

    limit_3Pin(limit_3),
    limit_4Pin(limit_4),
    limit_5Pin(limit_5),
    limit_6Pin(limit_6),
    limit_7Pin(limit_7),
    limit_8Pin(limit_8),

    collectUp(false),
    collectDown(false),
    collectRotate(false),
    handButton(false),

    lastHandButton(false),

    handState(OPEN),

    upLocked(false),
    downLocked(false),
    rotateLocked(false),

    recoveryState(RECOVERY_NONE),
    recoveryStartTime(0)
{
}


// ============================================================
// 初期化
// ============================================================

void Collect::begin()
{
  motor_3.begin();
  motor_4.begin();
  motor_5.begin();

  // STS3215
  Serial1.begin(1000000);
  hand.begin();

  // リミットスイッチ
  // INPUT_PULLUPなので
  // 押されていない = HIGH
  // 押されている   = LOW

  pinMode(limit_3Pin, INPUT_PULLUP);
  pinMode(limit_4Pin, INPUT_PULLUP);
  pinMode(limit_5Pin, INPUT_PULLUP);
  pinMode(limit_6Pin, INPUT_PULLUP);
  pinMode(limit_7Pin, INPUT_PULLUP);

  // Ls_8は現在使用しない
  // pinMode(limit_8Pin, INPUT_PULLUP);

  // 初期状態
  motor_3.stop();
  motor_4.stop();
  motor_5.stop();

  recoveryState = RECOVERY_NONE;

  upLocked = false;
  downLocked = false;
  rotateLocked = false;

  handState = OPEN;
  lastHandButton = false;
}


// ============================================================
// コントローラー入力
// ============================================================

// Bボタン
// true  = 上昇
// false = 上昇ボタンを離した
void Collect::setCollectUp(bool pressed)
{
  collectUp = pressed;
}


// Xボタン
// true  = 下降
// false = 下降ボタンを離した
void Collect::setCollectDown(bool pressed)
{
  collectDown = pressed;
}


// R1ボタン
// true  = 回転
// false = 回転停止
void Collect::setCollectRotate(bool pressed)
{
  collectRotate = pressed;
}


// Sボタン
// Hand開閉
void Collect::setHandButton(bool pressed)
{
  handButton = pressed;
}


// ============================================================
// メイン更新処理
// ============================================================

void Collect::update()
{
  // ----------------------------------------------------------
  // ① リミットからの退避中なら、通常操作より退避を優先
  // ----------------------------------------------------------

  if (recoveryState != RECOVERY_NONE)
  {
    updateRecovery();
    return;
  }


  // ----------------------------------------------------------
  // ② Hand処理
  // ----------------------------------------------------------

  updateHand();


  // ==========================================================
  // ③ 上昇・下降処理
  // ==========================================================

  // ----------------------------------------------------------
  // 上昇
  // Bが押されていて、上昇がロックされていない
  // ----------------------------------------------------------

  if (collectUp && !upLocked)
  {
    // 上昇方向のリミットを確認
    if (
      digitalRead(limit_3Pin) == LOW ||
      digitalRead(limit_5Pin) == LOW
    )
    {
      // 上昇側リミットに到達
      startRecoveryFromUpper();
      return;
    }

    // 上昇
    // motor_3 = -50
    // motor_4 = +50
    motor_3.drive(-MOTOR_3_SPEED);
    motor_4.drive(MOTOR_4_SPEED);
  }


  // ----------------------------------------------------------
  // 下降
  // Xが押されていて、下降がロックされていない
  // ----------------------------------------------------------

  else if (collectDown && !downLocked)
  {
    // 下降方向のリミットを確認
    if (
      digitalRead(limit_4Pin) == LOW ||
      digitalRead(limit_6Pin) == LOW
    )
    {
      // 下降側リミットに到達
      startRecoveryFromLower();
      return;
    }

    // 下降
    // motor_3 = +50
    // motor_4 = -50
    motor_3.drive(MOTOR_3_SPEED);
    motor_4.drive(-MOTOR_4_SPEED);
  }


  // ----------------------------------------------------------
  // 上昇も下降も押されていない
  // ----------------------------------------------------------

  else
  {
    motor_3.stop();
    motor_4.stop();
  }


  // ==========================================================
  // ④ 回転処理
  // ==========================================================

  if (collectRotate && !rotateLocked)
  {
    // Ls_7が押されているか確認
    if (digitalRead(limit_7Pin) == LOW)
    {
      // Ls_7に到達
      startRecoveryFromRotate();
      return;
    }

    // 回転
    motor_5.drive(MOTOR_5_SPEED);
  }
  else
  {
    motor_5.stop();
  }


  // ==========================================================
  // ⑤ リミットから離れた場合のロック解除
  // ==========================================================

  // 上昇側リミットから離れたら、
  // もう一度上昇できるようにする
  if (
    upLocked &&
    digitalRead(limit_3Pin) == HIGH &&
    digitalRead(limit_5Pin) == HIGH
  )
  {
    // ただし、すぐに解除するとリミット付近で
    // 上昇→リミット→退避を繰り返す可能性がある。
    // ここでは下降操作をした後に解除する。
  }


  // 下降操作をしたら上昇ロック解除
  if (collectDown)
  {
    upLocked = false;
  }


  // 上昇操作をしたら下降ロック解除
  if (collectUp)
  {
    downLocked = false;
  }


  // 回転については、Ls_7から離れる方向へ
  // 逆回転したときに解除する。
  //
  // 現在はmotor_5.drive(-50)を逆方向として扱う。
  //
  // この処理は後述のupdateRecovery()で行う。
}


// ============================================================
// 上昇側リミットに到達
// ============================================================

void Collect::startRecoveryFromUpper()
{
  // まず停止
  motor_3.stop();
  motor_4.stop();

  // 退避開始
  recoveryState = RECOVERY_FROM_UPPER;
  recoveryStartTime = millis();

  // 上昇方向をロック
  upLocked = true;
}


// ============================================================
// 下降側リミットに到達
// ============================================================

void Collect::startRecoveryFromLower()
{
  // まず停止
  motor_3.stop();
  motor_4.stop();

  // 退避開始
  recoveryState = RECOVERY_FROM_LOWER;
  recoveryStartTime = millis();

  // 下降方向をロック
  downLocked = true;
}


// ============================================================
// 回転側リミットに到達
// ============================================================

void Collect::startRecoveryFromRotate()
{
  // まず停止
  motor_5.stop();

  // 退避開始
  recoveryState = RECOVERY_FROM_ROTATE;
  recoveryStartTime = millis();

  // 回転方向をロック
  rotateLocked = true;
}


// ============================================================
// リミットからの退避処理
// ============================================================

void Collect::updateRecovery()
{
  unsigned long elapsed = millis() - recoveryStartTime;


  // ==========================================================
  // 上昇側リミットから退避
  // ==========================================================

  if (recoveryState == RECOVERY_FROM_UPPER)
  {
    // 下降方向へ少し戻す
    motor_3.drive(MOTOR_3_SPEED);
    motor_4.drive(-MOTOR_4_SPEED);

    if (elapsed >= RECOVERY_TIME)
    {
      motor_3.stop();
      motor_4.stop();

      recoveryState = RECOVERY_NONE;

      // 上昇は禁止したまま
      upLocked = true;

      // 下降は許可
      downLocked = false;
    }

    return;
  }


  // ==========================================================
  // 下降側リミットから退避
  // ==========================================================

  if (recoveryState == RECOVERY_FROM_LOWER)
  {
    // 上昇方向へ少し戻す
    motor_3.drive(-MOTOR_3_SPEED);
    motor_4.drive(MOTOR_4_SPEED);

    if (elapsed >= RECOVERY_TIME)
    {
      motor_3.stop();
      motor_4.stop();

      recoveryState = RECOVERY_NONE;

      // 下降は禁止したまま
      downLocked = true;

      // 上昇は許可
      upLocked = false;
    }

    return;
  }


  // ==========================================================
  // 回転側リミットから退避
  // ==========================================================

  if (recoveryState == RECOVERY_FROM_ROTATE)
  {
    // 逆方向へ少し戻す
    motor_5.drive(-MOTOR_5_SPEED);

    if (elapsed >= RECOVERY_TIME)
    {
      motor_5.stop();

      recoveryState = RECOVERY_NONE;

      // Ls_7に向かう方向の回転は禁止
      rotateLocked = true;
    }

    return;
  }
}


// ============================================================
// モーター停止
// ============================================================

void Collect::stopRack()
{
  motor_3.stop();
  motor_4.stop();
}


void Collect::stopRotate()
{
  motor_5.stop();
}


// ============================================================
// Hand制御
// ============================================================

void Collect::updateHand()
{
  // Sボタンを押した瞬間だけ動作させる
  //
  // 押しっぱなしで
  // OPEN → CLOSE → OPEN → CLOSE...
  // とならないようにする。

  if (handButton && !lastHandButton)
  {
    if (handState == OPEN)
    {
      hand.close();
      handState = CLOSE;
    }
    else
    {
      hand.open();
      handState = OPEN;
    }
  }

  // 今回の状態を次回用に保存
  lastHandButton = handButton;
}


// ============================================================
// Hand状態取得
// ============================================================

Collect::HandState Collect::getHandState()
{
  return handState;
}