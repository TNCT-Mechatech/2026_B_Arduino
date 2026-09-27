#include "TppSol.h"

// コンストラクタ：すべての設定値をメンバー変数に保存
TppSol::TppSol(int pwm_pin, int dir_pin, unsigned long shotTime, unsigned long shotLongTime, unsigned long interlockTime) {
  _pwm_pin = pwm_pin;
  _dir_pin = dir_pin;
  _shotTime = shotTime;
  _shotLongTime = shotLongTime;
  _interlockTime = interlockTime;
  _currentState = STATE_IDLE;
  _stateStartTime = 0;
  _activeShotTime = 0;
}

// 初期化：ピンを出力モードにして最初は完全に無通電(0%)にする
void TppSol::begin() {
  pinMode(_pwm_pin, OUTPUT);
  pinMode(_dir_pin, OUTPUT);
  analogWrite(_pwm_pin, 0);
  digitalWrite(_dir_pin, LOW);
}

// 通常のPUSH：_activeShotTime に通常の時間をセットして駆動
void TppSol::PUSH() {
  if (_currentState == STATE_IDLE) {
    _activeShotTime = _shotTime;  // 通常の時間を適用
    digitalWrite(_dir_pin, LOW); // モータードライバーの方向をPUSHへ
    analogWrite(_pwm_pin, 255);   // 100%フルパワー
    _stateStartTime = millis();
    _currentState = STATE_PUSH_SHOT;
  }
}

// ロングのPUSH：_activeShotTime に長い時間をセットして駆動（状態は通常と同じ）
void TppSol::PUSH_long() {
  if (_currentState == STATE_IDLE) {
    _activeShotTime = _shotLongTime; // ★ロング用の時間を適用
    digitalWrite(_dir_pin, LOW);    // モータードライバーの方向をPUSHへ
    analogWrite(_pwm_pin, 255);      // 100%フルパワー
    _stateStartTime = millis();
    _currentState = STATE_PUSH_SHOT; // 状態は通常のPUSH_SHOTと共通でOK！
  }
}

// PULL（引き込み）：通常の時間で引き戻す
void TppSol::PULL() {
  if (_currentState == STATE_IDLE) {
    _activeShotTime = _shotTime;  // 引き込みは通常の時間を適用（機構に合わせて要調整）
    digitalWrite(_dir_pin, HIGH);  // モータードライバーの方向をPULLへ
    analogWrite(_pwm_pin, 255);   // 100%フルパワー
    _stateStartTime = millis();
    _currentState = STATE_PULL_SHOT;
  }
}

// 状態機械の定期更新（loop内で毎回必ず呼ぶ）
void TppSol::update() {
  unsigned long currentTime = millis();

  switch (_currentState) {
    case STATE_IDLE:
      // 通電は0%。永久磁石の力で保持中。
      break;

    case STATE_PUSH_SHOT:
    case STATE_PULL_SHOT:
      // PUSH/PUSH_long/PULLのいずれかでセットされた「_activeShotTime」を超えたら通電停止
      if (currentTime - _stateStartTime >= _activeShotTime) {
        analogWrite(_pwm_pin, 0); // 通電を完全に停止！(0%)
        _stateStartTime = currentTime;
        _currentState = STATE_INTERLOCK; // 連打防止（お休み）状態へ
      }
      break;

    case STATE_INTERLOCK:
      // コイルを休ませつつ、次の命令を弾く安全時間
      if (currentTime - _stateStartTime >= _interlockTime) {
        _currentState = STATE_IDLE; // 待機状態に戻り、次の入力を解禁
      }
      break;
  }
}

// 現在の状態を確認する関数
TppSol::State TppSol::getState() {
  return _currentState;
}