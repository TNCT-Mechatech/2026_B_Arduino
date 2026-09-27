#ifndef COLLECT_H
#define COLLECT_H

#include <Arduino.h>
#include "Motor.h"
#include "Hand.h"

class Collect
{
public:
  Collect(
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
  );

  void begin();
  void update();

  // コントローラー入力
  void setCollectUp(bool pressed);
  void setCollectDown(bool pressed);
  void setCollectRotate(bool pressed);
  void setHandButton(bool pressed);

  enum HandState
  {
    OPEN,
    CLOSE
  };

  HandState getHandState();

private:

  Motor motor_3;
  Motor motor_4;
  Motor motor_5;

  Hand hand;

  // リミットスイッチ
  uint8_t limit_3Pin;
  uint8_t limit_4Pin;
  uint8_t limit_5Pin;
  uint8_t limit_6Pin;
  uint8_t limit_7Pin;
  uint8_t limit_8Pin;   // 現在未使用

  // コントローラー
  bool collectUp;
  bool collectDown;
  bool collectRotate;
  bool handButton;

  // Handボタンの前回状態
  bool lastHandButton;

  // Hand状態
  HandState handState;

  // 上昇・下降のロック
  bool upLocked;
  bool downLocked;

  // 回転のロック
  bool rotateLocked;

  // リミットからの退避
  enum RecoveryState
  {
    RECOVERY_NONE,
    RECOVERY_FROM_UPPER,
    RECOVERY_FROM_LOWER,
    RECOVERY_FROM_ROTATE
  };

  RecoveryState recoveryState;

  unsigned long recoveryStartTime;

  // リミットから離れるための時間
  static const unsigned long RECOVERY_TIME = 100;

  // モーター速度
  static const int MOTOR_3_SPEED = -150;
  static const int MOTOR_4_SPEED = 150;
  static const int MOTOR_5_SPEED = -50;

  // モーター停止
  void stopRack();
  void stopRotate();

  // 退避処理
  void startRecoveryFromUpper();
  void startRecoveryFromLower();
  void startRecoveryFromRotate();
  void updateRecovery();

  // Hand処理
  void updateHand();
};

#endif