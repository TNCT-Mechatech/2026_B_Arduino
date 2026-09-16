#include "Collect.h"
//0916

Collect ::Collect(
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

    hand(21),   //sts3215のID21に固定

    limit_3Pin(limit_3),
    limit_4Pin(limit_4),
    limit_5Pin(limit_5),
    limit_6Pin(limit_6),
    limit_7Pin(limit_7),
    limit_8Pin(limit_8),

   

    controllerCollect(false),

    rackState(RACK_HIGH),
    handState(OPEN)
{
}

void Collect::begin()
{
  motor_3.begin();
  motor_4.begin();
  motor_5.begin();

  Serial1.begin(1000000);
  hand.begin();

  pinMode(limit_3Pin, INPUT_PULLUP);
  pinMode(limit_4Pin, INPUT_PULLUP);
  pinMode(limit_5Pin, INPUT_PULLUP);
  pinMode(limit_6Pin, INPUT_PULLUP);
  pinMode(limit_7Pin, INPUT_PULLUP);
  pinMode(limit_8Pin, INPUT_PULLUP);



  motor_3.stop();
  motor_4.stop();
  motor_5.stop();
}
// コントローラーXによる回収命令
void Collect::setControllerCollect(bool pressed)
{
  controllerCollect = pressed;
}

void Collect::update()
{
  //(状態)HIGH　かつ　OPEN--------------------------------------------------------
  if(rackState == RACK_HIGH &&
   handState == OPEN &&
   controllerCollect)
{
  //(状態遷移)DOWN
  rackState = RACK_DOWN;
}

  //(状態)DOWN--------------------------
  if(rackState == RACK_DOWN)
  {
    if(digitalRead(limit_4Pin) == HIGH)
    {
      motor_3.drive(50);
    }
    else
    {
      motor_3.stop();
    }

    if(digitalRead(limit_6Pin) == HIGH)
    {
      motor_4.drive(50);
    }
    else
    {
      motor_4.stop();
    }

    if(digitalRead(limit_4Pin) == LOW && digitalRead(limit_6Pin) == LOW && rackState == RACK_DOWN)
    {
    //(状態遷移)ラック最下部
    rackState = RACK_LOW;
    }
  }
  
  //(状態)RACK_LOW----------
  if(rackState == RACK_LOW)
  {
    if(handState == OPEN)
    {
      hand.close();
      handState = CLOSE;
    }

    if(handState == CLOSE)
    {
      //(状態遷移)上昇中
      rackState = RACK_UP;
    }
  }

  //(状態)RACK_UP かつ　CLOSE
  if(rackState == RACK_UP)
  {
    if(digitalRead(limit_3Pin) == HIGH)
    {
      motor_3.drive(-50);
    }
    else
    {
      motor_3.stop();
    }

    if(digitalRead(limit_5Pin) == HIGH)
    {
      motor_4.drive(-50);
    }
    else
    {
      motor_4.stop();
    }
    
    if(digitalRead(limit_3Pin) == LOW && digitalRead(limit_5Pin) == LOW && rackState == RACK_UP)
    {
      //(状態遷移)ANGLE　CLOSE 
      rackState = RACK_ANGLE;
      motor_3.stop();
      motor_4.stop();
    }
  }

  //(状態)ANGLE かつ CLOSE---------------------
  if(rackState == RACK_ANGLE && handState == CLOSE)
  {
    //補充機構の上についたか？
    if(digitalRead(limit_8Pin) == HIGH)
    {
      motor_5.drive(50);  //dir1
    }
    else
    {
      motor_5.stop();
      //(状態遷移)RELOAD
      rackState = RACK_RELOAD;
    }
  }

  //(状態)RELOAD かつ　CLOSE--------------------------
  if(rackState == RACK_RELOAD && handState == CLOSE)
  {
    if(handState == CLOSE)
    {
      hand.open();
      //(状態遷移)RELOAD OPEN
      handState = OPEN;
    }
  }

  //(状態)RELOAD かつ OPEN----------------------------
  if(rackState == RACK_RELOAD && handState == OPEN)
  {
    //(状態遷移)ANGLE OPEN
    rackState = RACK_ANGLE;
  }

  //(状態)ANGLE かつ OPEN---------------------------
  if(rackState == RACK_ANGLE && handState == OPEN)
  {
    //角度移動バケツの上についたか？
    if(digitalRead(limit_7Pin) == HIGH)
    {
      motor_5.drive(-50); //
    }
    else
    {
      motor_5.stop();
      //(状態遷移)HIGI OPEN
      rackState = RACK_HIGH;
    }
  } 
}

Collect::RackState Collect::getRackState()
{
  return rackState;
}

Collect::HandState Collect::getHandState()
{
  return handState;
}