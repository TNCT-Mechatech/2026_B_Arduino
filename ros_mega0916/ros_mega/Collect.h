#ifndef COLLECT_H
#define COLLECT_H

#include <Arduino.h>
#include "Motor.h"
#include "Hand.h"

//0825

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
 void setControllerCollect(bool pressed);
  //状態
  enum RackState
  {
    RACK_HIGH,  //  
    RACK_DOWN,
    RACK_UP,
    RACK_LOW,
    RACK_ANGLE,
    RACK_RELOAD
  };

  enum HandState
  {
    OPEN,
    CLOSE
  };

  RackState getRackState();
  HandState getHandState();

private:
  Motor motor_3;
  Motor motor_4;
  Motor motor_5;

  Hand hand;  //調整箇所③　サーボモータのポート番号

  uint8_t limit_3Pin;
  uint8_t limit_4Pin;
  uint8_t limit_5Pin;
  uint8_t limit_6Pin;
  uint8_t limit_7Pin;
  uint8_t limit_8Pin;

  
 bool controllerCollect = false;

  RackState rackState;
  HandState handState;

  static const int MOTOR_3_SPEE = 200;  //  調整箇所② 回収機構の各モータ速度0~255
  static const int MOTOR_4_SPEE = 200;
  static const int MOTOR_5_SPEE = 200;
};

#endif