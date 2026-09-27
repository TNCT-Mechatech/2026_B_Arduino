#include "Omni.h"

#include <math.h>


//==================================================
// コンストラクタ
//==================================================

Omni::Omni(
    uint8_t pwmPin1,
    uint8_t dirPin1,

    uint8_t pwmPin2,
    uint8_t dirPin2,

    uint8_t pwmPin3,
    uint8_t dirPin3,

    uint8_t pwmPin4,
    uint8_t dirPin4
)
    : motor1(pwmPin1, dirPin1),
      motor2(pwmPin2, dirPin2),
      motor3(pwmPin3, dirPin3),
      motor4(pwmPin4, dirPin4)
{
    //==================================================
    // オムニ制御
    //==================================================

    omega = 0.0;
    r = 0.0;
    theta = 0.0;

    ROT_GAIN = 0.4;

    //感度調整
    MOVE_CURVE = 3.0f;
    ROT_CURVE  = 3.0f;
}


//==================================================
// 初期化
//==================================================

void Omni::begin()
{
    //==================================================
    // オムニモータ
    //==================================================

    motor1.begin();
    motor2.begin();
    motor3.begin();
    motor4.begin();


    //==================================================
    // 初期値
    //==================================================

    drive(
        0,
        0,
        0,
        0
    );
}


//==================================================
// 入力カーブ
//==================================================

float Omni::curveInput(
    float input,
    float exponent)
{
    return copysign(
        pow(
            abs(input),
            exponent
        ),
        input
    );
}


//==================================================
// ジョイスティック入力
//==================================================

void Omni::update(
    int joyX,
    int joyY,
    int joyRot)
{
    //==================================================
    // ジョイスティック入力
    //==================================================

    float inputX =
        joyX / 255.0f;

    float inputY =
        joyY / 255.0f;

    float inputRot =
        joyRot / 255.0f;


    //==================================================
    // 移動量
    //==================================================

    r =
        sqrt(
            (long)joyX * joyX +
            (long)joyY * joyY
        );


    theta =
        atan2(
            joyY,
            joyX
        );


    //==================================================
    // 移動カーブ
    //==================================================

    float moveInput =
        r / 255.0f;

    moveInput =
        pow(
            moveInput,
            MOVE_CURVE
        );

    r =
        moveInput * 255.0f;


    //==================================================
    // 旋回カーブ
    //==================================================

    float rotInput =
        curveInput(
            inputRot,
            ROT_CURVE
        );

    omega =
        rotInput * 255.0f;


    //==================================================
    // オムニ運動学
    //==================================================

    // 【長方形への修正】
    // 正方形(844mm)から長方形(L+W = 387+442 = 829mm)に変わったための寸法補正を入れます
    float adjusted_rot_gain = ROT_GAIN * (829.0f / 844.0f);

    float out1 =
        r * sin(theta + PI / 4.0)
        - adjusted_rot_gain * omega;


    float out2 =
        -r * cos(theta + PI / 4.0)
        - adjusted_rot_gain * omega;


    float out3 =
        -r * sin(theta + PI / 4.0)
        - adjusted_rot_gain * omega;

    float out4 =
        r * cos(theta + PI / 4.0)
        - adjusted_rot_gain * omega;


    /*正方形
    float out1 =
        r * sin(theta + PI / 4.0)
        - ROT_GAIN * omega;


    float out2 =
        -r * cos(theta + PI / 4.0)
        - ROT_GAIN * omega;


    float out3 =
        -r * sin(theta + PI / 4.0)
        - ROT_GAIN * omega;


    float out4 =
        r * cos(theta + PI / 4.0)
        - ROT_GAIN * omega;
    */

    //==================================================
    // スケーリング
    //==================================================

    float maxVal =
        max(
            abs(out1),
            max(
                abs(out2),
                max(
                    abs(out3),
                    abs(out4)
                )
            )
        );


    if(maxVal > 255.0)
    {
        float scale =
            255.0 / maxVal;

        out1 *= scale;
        out2 *= scale;
        out3 *= scale;
        out4 *= scale;
    }


    //==================================================
    // オムニモータ出力
    //==================================================

    drive(
        (int)out1,
        (int)out2,
        (int)out3,
        (int)out4
    );
}


//==================================================
// オムニモータ drive
//==================================================

void Omni::drive(
    int out1,
    int out2,
    int out3,
    int out4)
{
    motor1.drive(out1);
    motor2.drive(out2);
    motor3.drive(out3);
    motor4.drive(out4);
}