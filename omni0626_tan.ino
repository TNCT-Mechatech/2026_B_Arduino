//オムニ制御修正版_0626
//運動学の修正
#include <math.h>
// ==================================================================
// MX1508モータードライバーの制御クラス
// =================================================================
// モータードライバー抽象化クラスの定義（MX1508専用仕様） ライブラリのコピペ
// int speed に -255〜255 の値を入れると、正転・逆転・停止を自動判定してくれる
// motor << MX1508 << Arduino Uno 専用
class Motor {
  public:
    virtual void drive(int speed) = 0;
};

// 【MX1508用：PWMピン節約版】PWMピン1本 + デジタルピン1本で動かすモード
class MX1508Motor : public Motor {
  private:
    int _pwmPin; // 必ずArduinoの「~」マークのピンに接続（MX1508のIN1へ）
    int _digPin; // 任意の通常デジタルピンでOK（MX1508のIN2へ）
    bool _reverse;
  public:
    MX1508Motor(long pwmPin, int digPin, bool reverse = false) {
      _pwmPin = pwmPin; _digPin = digPin; _reverse = reverse;
      pinMode(_pwmPin, OUTPUT); pinMode(_digPin, OUTPUT);
    }
    void drive(int speed) override {
      if (_reverse) speed = -speed;
      
      if (speed > 0) {  //正回転
        // 【正転】IN1(PWM)に速度、IN2(DIG)をLOWにする
        analogWrite(_pwmPin, speed);
        digitalWrite(_digPin, LOW);
      } else if (speed < 0) { //逆回転
        // 【逆転】IN1(PWM)をHIGH固定し、IN2(DIG)に反転PWMを入力（差分駆動）
        int revSpeed = abs(speed);
        analogWrite(_pwmPin, 255 - revSpeed);
        digitalWrite(_digPin, HIGH);
      } else {
        // 【停止】両方LOW
        digitalWrite(_pwmPin, LOW);
        digitalWrite(_digPin, LOW);
      }
    }
};

// pin定義
const int PIN_X = A3; //左joy X軸
const int PIN_Y = A2; //左joy Y軸
const int PIN_ROT = A5; //右joy X軸（旋回用）
const int DEADZONE = 30; //デッドゾーンの設定

// 変数の宣言
int joyX, joyY, joyRot; //ジョイスティックの値を格納する変数
float omega = 0.0; //旋回速度の変数ω
float r = 0.0; //移動速度変数r
float theta = 0.0; //移動方向の変数Θ
float L = 0.6; //旋回速度の係数（大きいほど旋回が速くなる）


// =================================================================
// モーターの割り当てとインスタンス化//m1(3,2,true)で逆転

MX1508Motor m1(3, 2,true);          // 左前 (IN1=3, IN2=2)
MX1508Motor m2(6, 7);          // 左後 (IN1=6, IN2=7)
MX1508Motor m3(9, 8);          // 右後 (IN1=9, IN2=8)
MX1508Motor m4(5, 4,true);          // 右前 (IN1=5, IN2=4)

// =================================================================
//メイン処理（Setup & Loop）
void setup() {
  // 0, 1番ピン（RX/TX）を回避しているため、シリアルデバッグも安全に使用可能
  //Serial.begin(115200); 
}

void loop() {
  joy(PIN_X, PIN_Y, PIN_ROT);// joy関数でジョイスティックの値を取得

  // 速度と方向の計算
  r = sqrt((long)joyX * joyX + (long)joyY * joyY); // 移動速度rの計算（ベクトルの大きさ）
  theta = atan2(joyY, joyX); // 移動方向Θの計算 0~360
  omega = joyRot; // 旋回速度ωの計算（右ジョイスティックのX軸）
  r = min(r, 255.0); // 移動速度rを最大値255に制限
  omega = constrain(omega, -255, 255); // 旋回速度ωを-255〜255に制限

  // 4輪運動学計算（omega+theta+rをモータ4つに変換）
  int out1 = r * sin(theta + PI / 4) - L * omega; // 左前
  int out4 = r * cos(theta + PI / 4) - L * omega; // 右前
  int out2 = r * cos(theta + PI / 4) + L * omega; // 左後
  int out3 = r * sin(theta + PI / 4) + L * omega; // 右後

  // 出力の比率を維持したまま、最大値を255に丸める（斜め移動時の方向破綻を防ぐ）
  int maxVal = max(abs(out1), max(abs(out2),
               max(abs(out3), abs(out4))));
  if (maxVal > 255) {
    float scale = 255.0 / maxVal;
    out1 *= scale;
    out2 *= scale;
    out3 *= scale;
    out4 *= scale;
  }
  //mdebug(out1,out2,out3,out4);

  // 各MX1508へ信号を出力
  m1.drive(out1);
  m2.drive(out2);
  m3.drive(out3);
  m4.drive(out4);

  //debug();
  

  delay(20); // 制御周期の安定化
}

// =================================================================
// joy関数
void joy(int pinX, int pinY, int pinRot) {
  // ジョイスティックの値を読み込み (-255 〜 255 にマッピング)
  joyX = -map(analogRead(pinX), 0, 1023, -255, 255);
  joyY = map(analogRead(pinY), 0, 1023, -255, 255);
  joyRot = map(analogRead(pinRot), 0, 1023, -255, 255);

  // 不感帯処理（中心付近の微小なブレを無視して完全に止める）
  if (abs(joyX) < DEADZONE) joyX = 0;
  if (abs(joyY) < DEADZONE) joyY = 0;
  if (abs(joyRot) < DEADZONE) joyRot = 0;
}
// =================================================================
// デバッグ用関数
void debug() {
  Serial.print("joyX: ");
  Serial.print(joyX);

  Serial.print("\tjoyY: ");
  Serial.print(joyY);

  Serial.print("\tjoyRot: ");
  Serial.print(joyRot);

  Serial.print("\tr: ");
  Serial.print(r);

  Serial.print("\ttheta: ");
  Serial.print(theta);

  Serial.print("\tomega: ");
  Serial.println(omega);
}
void mdebug(int out1, int out2, int out3, int out4){
  
  Serial.print("m1: ");
  Serial.print(out1);
  Serial.print(" m2: ");
  Serial.print(out2);
  Serial.print(" m3: ");
  Serial.print(out3);
  Serial.print(" m4: ");
  Serial.print(out4);

}