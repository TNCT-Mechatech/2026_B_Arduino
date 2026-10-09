#ifndef COLLECT_H
#define COLLECT_H

#include <Arduino.h>
#include "Motor.h"
#include "Hand.h"

class Collect
{
public:

  // ==================================================
  // コンストラクタ
  // ==================================================
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
    uint8_t limit_7
  );


  // ==================================================
  // 基本
  // ==================================================
  void begin();
  void update();


  // ==================================================
  // 入力設定
  // ==================================================
  void setCollectUp(bool pressed);
  void setCollectDown(bool pressed);
  void setCollectRotate(bool pressed);
  void setCollectBack(bool pressed);
  void setHandButton(bool pressed);


  // ==================================================
  // ハンド状態
  // ==================================================
  enum HandState
  {
    OPEN,
    CLOSE
  };

  HandState getHandState();


private:

  // ==================================================
  // モーター
  // ==================================================
  Motor motor_3;
  Motor motor_4;
  Motor motor_5;

  Hand hand;


  // ==================================================
  // リミットスイッチ
  //
  // limit_3 = M3 下限
  // limit_4 = M3 上限
  // limit_5 = M4 上限
  // limit_6 = M4 下限
  // limit_7 = M5 回転停止
  // ==================================================
  uint8_t limit_3Pin;
  uint8_t limit_4Pin;
  uint8_t limit_5Pin;
  uint8_t limit_6Pin;
  uint8_t limit_7Pin;


  // ==================================================
  // 入力状態
  // ==================================================
  bool collectUp;
  bool collectDown;

  bool collectRotate;
  bool collectBack;

  bool handButton;


  // ==================================================
  // 前回状態
  // ==================================================
  bool lastCollectRotate;
  bool lastHandButton;


  // ==================================================
  // ハンド
  // ==================================================
  HandState handState;


  // ==================================================
  // M3/M4 方向ロック
  // ==================================================

  // 上方向ロック
  bool motor_3upLocked;
  bool motor_4upLocked;

  // 下方向ロック
  bool motor_3downLocked;
  bool motor_4downLocked;


  // ==================================================
  // M5 回転状態
  // ==================================================
  enum RotateState
  {
    ROTATE_STOP,
    ROTATE_AUTO
  };

  RotateState rotateState;


  // ==================================================
  // M3/M4 退避状態
  // ==================================================
  enum MotorRecoveryState
  {
    RECOVERY_NONE,
    RECOVERY_FROM_UPPER,
    RECOVERY_FROM_LOWER
  };


  // ==================================================
  // M3 専用
  // ==================================================
  MotorRecoveryState motor_3RecoveryState;
  unsigned long motor_3RecoveryStartTime;


  // ==================================================
  // M4 専用
  // ==================================================
  MotorRecoveryState motor_4RecoveryState;
  unsigned long motor_4RecoveryStartTime;


  // ==================================================
  // 退避時間
  // ==================================================
  static const unsigned long RECOVERY_TIME = 100;


  // ==================================================
  // モーター速度
  // ==================================================
  static const int MOTOR_3_SPEED = 100;
  static const int MOTOR_4_SPEED = 150;
  static const int MOTOR_5_SPEED = 150;


  // ==================================================
  // 停止
  // ==================================================
  void stopRack();
  void stopRotate();


  // ==================================================
  // M3 退避
  // ==================================================
  void startMotor3RecoveryFromUpper();
  void startMotor3RecoveryFromLower();
  void updateMotor3Recovery();


  // ==================================================
  // M4 退避
  // ==================================================
  void startMotor4RecoveryFromUpper();
  void startMotor4RecoveryFromLower();
  void updateMotor4Recovery();


  // ==================================================
  // その他
  // ==================================================
  void updateRotate();
  void updateHand();
};

#endif