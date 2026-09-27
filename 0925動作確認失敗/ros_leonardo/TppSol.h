//TppSol.h
//TPPソレノイドクラス
//Hブリッジ回路の代わりに「MD10C」や「MD13S」を使用しPWMとDirで制御
//
//「タカハの12V用CG0836の双方向自己保持型TPPソレノイド」に使用する
//robocon26B
//kanayama shunya

#ifndef TPP_SOL_H
#define TPP_SOL_H

#include <Arduino.h>

class TppSol {
  public:
    // 状態の定義（電気的な名前で共通化）
    enum State {
      STATE_IDLE,       //無通電キープ・入力待機
      STATE_PUSH_SHOT,  //押し出し稼働（フルパワー・プッシュ・短時間）
      STATE_PULL_SHOT,  //引き込み稼働（フルパワー・プル・短時間）
      STATE_INTERLOCK   //通電完全停止・連打防止
    };

  private:
    int _pwm_pin;                 // 制御するPWMピン番号
    int _dir_pin;                 // 制御するdirピン番号 (既製品の場合：HIGH:PUSH / LOW:PULL) >> 配線でプラス・マイナスを逆にしている場合(HIGH:PULL / LOW:PUSH)
    unsigned long _shotTime;      // PUSH/PULL時に通電する時間 (ms)
    unsigned long _shotLongTime;  // ロング用の通電時間（ms）
    unsigned long _interlockTime;  // 冷却・復帰する時間 (ms)
    
    State _currentState;          // 現在の状態
    unsigned long _stateStartTime;// 状態が始まった時刻
    unsigned long _activeShotTime;// 現在適用されている通電時間（ms）

  public:
    // コンストラクタ（初期設定の受け取り口）
    TppSol(int pwm_pin, int dir_pin, unsigned long shotTime, unsigned long shotLongTime, unsigned long interlockTime);

    void begin();    // 初期化処理
    void PUSH();     //　PUSH（プッシュ（押し出し））
    void PUSH_long();//  PUSHの押し出し稼働時間が長い
    void PULL();     //　PULL（プル（吸引，引き込み））
    void update();   // 状態機械の更新処理（loop内で毎回呼ぶ）

    State getState(); // 現在の状態を外から確認する関数
};

#endif