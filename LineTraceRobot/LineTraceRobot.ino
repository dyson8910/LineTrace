#include <SoftwareSerial.h>

//Bluetoothモジュール
const int BtRx = 13; //受信用ピン
const int BtTx = 12; //送信用ピン
SoftwareSerial btSerial(BtRx, BtTx); //Bluetooth用シリアル通信設定

//タクトスイッチ
const int Switch = 11;

//フォトリフレクタ
const int Sensor_r = 2;
const int Sensor_c = 1;
const int Sensor_l = 0;
const int thresh = 930; //フォトリフレクタの閾値

//モーター
const int motorRpwm = 5;
const int motorR1 = 4;
const int motorR2 = 3;

const int motorLpwm = 9;
const int motorL1 = 8;
const int motorL2 = 7;

/*
  状態
  0:待機
  1:受信待機
  2:実行
*/
int state = 0;

//制御パラメータ
double param[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

String text = "";
char mode[7] = "";
char bufforp[15][10] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};

int intention = 0;
int p_intention = 0;

//文字列を送信する関数
void TransString(String txt) {
  int i = 0;
  for (i = 0; i < txt.length(); i++) {
    btSerial.write(txt.charAt(i));
  }
}

//文字列を受信する関数
String ReceiveString() {
  String txt = "";
  while (btSerial.available()) {
    delay(50);
    char inputchar = btSerial.read();
    txt = String(txt + inputchar);
  }
  return txt;
}

//スイッチが押されたか確認する関数
//スイッチが押されていると1,押されていなければ0
int SwitchCheck() {
  if (digitalRead(Switch) == LOW) {
    delay(50); //チャタリング防止
    if (digitalRead(Switch) == LOW) {
      return 1;
    }
  }
  return 0;
}

//フォトリフレクタの値が閾値を超えているか確認する関数
//閾値より大きい=黒=1、小さい=白=0
int Sensor(int pin) {
  int v = analogRead(pin);
  if (v > thresh) {
    return 1;
  } else {
    return 0;
  }
}

//フォトトランジスタの値からコースへの偏差を計算する関数
int PhotoCheck() {
  int right = Sensor(Sensor_r);
  int center = Sensor(Sensor_c);
  int left = Sensor(Sensor_l);
  int sensor = 4 * left + 2 * center + 1 * right;

  switch (sensor) {
    case 0:
      return 10;
      break;
    case 1:
      return -2;
      break;
    case 2:
      return 0;
      break;
    case 3:
      return -1;
      break;
    case 4:
      return 2;
      break;
    case 5:
      return 7;
      break;
    case 6:
      return 1;
      break;
    case 7:
      return 100;
      break;
  }
}

//ONOFF制御実行関数
void ONOFFexe(double* Ks) {
  //開始時刻
  unsigned long stime = millis();
  //左右のpwm
  int l_pwm = 0;
  int r_pwm = 0;
  //スピード
  double def_speed = Ks[0];

  digitalWrite(motorR1, HIGH);
  digitalWrite(motorR2, LOW);
  digitalWrite(motorL1, HIGH);
  digitalWrite(motorL2, LOW);

  int check = 0;
  //
  while (1) {
    //スイッチ押すと強制終了
    if (SwitchCheck()) {
      digitalWrite(motorR1, LOW);
      digitalWrite(motorR2, LOW);
      digitalWrite(motorL1, LOW);
      digitalWrite(motorL2, LOW);
      delay(50);
      break;
    }
    //センサ値を格納
    p_intention = intention;
    intention = PhotoCheck();

    switch (intention) {
      case 100://ゴール
        //経過時間を返す
        {
          unsigned long etime = millis();
          long resultms = etime - stime;
          long results = resultms / 1000;
          resultms = resultms % 1000;
          long resultm = results / 60;
          results = results % 60;
          char result[30];
          sprintf(result, "0:%ld:%ld.%03d", resultm, results, resultms);
          TransString(result);

          //タイヤを止める
          digitalWrite(motorR1, LOW);
          digitalWrite(motorR2, LOW);
          digitalWrite(motorL1, LOW);
          digitalWrite(motorL2, LOW);
          delay(50);
          check = 1;
          break;
        }
      case 10://□□□ コースアウト Ks1,2
        r_pwm = int(def_speed * Ks[1] / 100.0);
        l_pwm = int(def_speed * Ks[2] / 100.0);
        break;
      case -2://□□■Ks3,4
        r_pwm = int(def_speed * Ks[3] / 100.0);
        l_pwm = int(def_speed * Ks[4] / 100.0);
        break;
      case -1://□■■Ks9,10
        r_pwm = int(def_speed * Ks[9] / 100.0);
        l_pwm = int(def_speed * Ks[10] / 100.0);
        break;
      case 0://□■□Ks5,6
        r_pwm = int(def_speed * Ks[5] / 100.0);
        l_pwm = int(def_speed * Ks[6] / 100.0);
        break;
      case 1://■■□Ks13,14
        r_pwm = int(def_speed * Ks[13] / 100.0);
        l_pwm = int(def_speed * Ks[14] / 100.0);
        break;
      case 2://■□□Ks7,8
        r_pwm = int(def_speed * Ks[7] / 100.0);
        l_pwm = int(def_speed * Ks[8] / 100.0);
        break;
      case 7://■□■Ks11,12
        r_pwm = int(def_speed * Ks[11] / 100.0);
        l_pwm = int(def_speed * Ks[12] / 100.0);
        break;
      default:
        r_pwm = 0;
        l_pwm = 0;
        break;
    }

    if (check) {
      break;//終了
    } else {
      if (l_pwm >= 0) {
        digitalWrite(motorL1, HIGH);
        digitalWrite(motorL2, LOW);
        analogWrite(motorLpwm, l_pwm);
      } else {
        digitalWrite(motorL1, LOW);
        digitalWrite(motorL2, HIGH);
        analogWrite(motorLpwm, -l_pwm);
      }
      if (r_pwm >= 0) {
        digitalWrite(motorR1, HIGH);
        digitalWrite(motorR2, LOW);
        analogWrite(motorRpwm, r_pwm);
      } else {
        digitalWrite(motorR1, LOW);
        digitalWrite(motorR2, HIGH);
        analogWrite(motorRpwm, -r_pwm);
      }
      delay(100);
    }
  }

}

//PID制御実行関数
void PIDexe(double* Ks) {
  //開始時刻
  unsigned long stime = millis();
  unsigned long now = millis();
  unsigned long before = millis();
  //左右のpwm
  int l_pwm = 0;
  int r_pwm = 0;
  //Ks0-speed Ks1-P Ks2-I Ks3-D
  double def_speed = Ks[0];
  double Kp = Ks[1];
  double Ki = Ks[2];
  double Kd = Ks[3];
  int r_gain, l_gain;
  digitalWrite(motorR1, HIGH);
  digitalWrite(motorR2, LOW);
  digitalWrite(motorL1, HIGH);
  digitalWrite(motorL2, LOW);

  int check = 0;
  while (1) {
    //スイッチ押すと強制終了
    if (SwitchCheck()) {
      digitalWrite(motorR1, LOW);
      digitalWrite(motorR2, LOW);
      digitalWrite(motorL1, LOW);
      digitalWrite(motorL2, LOW);
      delay(50);
      break;
    }

    //センサ値格納;
    p_intention = intention;
    intention = PhotoCheck();

    if (intention == 7) {
      intention = 0;
    }
    if (((p_intention == 1) || (p_intention == 2) || (p_intention == 3)) && (intention == 10)) {
      intention = 3;
    }
    if (((p_intention == -1) || (p_intention == -2) || (p_intention == -3)) && (intention == 10)) {
      intention = -3;
    }
    if (intention == 10) {
      intention = 0;
    }

    if (intention == 100) { //ゴール
      //経過時間を返す
      unsigned long etime = millis();
      long resultms = etime - stime;
      long results = resultms / 1000;
      resultms = resultms % 1000;
      long resultm = results / 60;
      results = results % 60;
      char result[30];
      sprintf(result, "0:%ld:%ld.%03d", resultm, results, resultms);
      TransString(result);

      //タイヤを止める
      digitalWrite(motorR1, LOW);
      digitalWrite(motorR2, LOW);
      digitalWrite(motorL1, LOW);
      digitalWrite(motorL2, LOW);
      delay(50);
      check = 1;
      break;
    } else { //ゴールしてない
      before = now;
      now = millis();
      double P = intention;
      double I = (intention + p_intention) / 2.0;
      double D = intention - p_intention;
      double gain = Kp * P + Ki * I + Kd * D;
      if (gain >= 0) {
        r_gain = int(def_speed);
        l_gain = int(def_speed - gain);
      } else {
        r_gain = int(def_speed + gain);
        l_gain = int(def_speed);
      }
      if (r_gain < 0) {
        r_gain = 0;
      }
      if (l_gain < 0) {
        l_gain = 0;
      }
    }
    digitalWrite(motorR1, HIGH);
    digitalWrite(motorR2, LOW);
    digitalWrite(motorL1, HIGH);
    digitalWrite(motorL2, LOW);
    analogWrite(motorRpwm, r_gain);
    analogWrite(motorLpwm, l_gain);
    delay(100);
  }
}

void setup() {
  btSerial.begin(15200); //Bluetoothシリアル開始
  pinMode(Switch, INPUT_PULLUP); //プルアップ抵抗

  //モーターのピンを初期化
  pinMode(motorR1, OUTPUT);
  pinMode(motorR2, OUTPUT);
  pinMode(motorL1, OUTPUT);
  pinMode(motorL2, OUTPUT);
  digitalWrite(motorR1, LOW);
  digitalWrite(motorR2, LOW);
  digitalWrite(motorL1, LOW);
  digitalWrite(motorL2, LOW);
}

void loop() {
  switch (state) {
    case 0://待機
      if (SwitchCheck() == 1) {
        delay(500);
        TransString(String("Ready"));
        state = 1;
      }
      break;
    case 1://受信待機
      if (SwitchCheck() == 1) { //エラー時はスイッチを押してリセット
        state = 0;
      } else if (btSerial.available()) { //受信データ有り
        text = ReceiveString();
        char str[128] = "";
        strcpy(str, text.c_str());
        int i;
        state = 2;
        for (i = 0; i < 7; i++) {
          mode[i] = 0;
        }
        if (text.charAt(0) == 'O') {
          sscanf(str, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", mode, bufforp[0], bufforp[1], bufforp[2], bufforp[3], bufforp[4], bufforp[5], bufforp[6], bufforp[7], bufforp[8], bufforp[9], bufforp[10], bufforp[11], bufforp[12], bufforp[13], bufforp[14]);
          for (i = 0; i < 15; i++) {
            param[i] = atof(bufforp[i]);
          }
        } else if (text.charAt(0) == 'P') {
          sscanf(str, "%s%s%s%s%s", mode, bufforp[0], bufforp[1], bufforp[2], bufforp[3]);
          for (i = 0; i < 4; i++) {
            param[i] = atof(bufforp[i]);
          }
        }
        delay(1000);
        TransString(String("Success"));
      }
      break;
    case 2://実行　//TODO
      delay(1000);
      if (strncmp(mode, "ON-OFF", 6)) {
        ONOFFexe(param);
      } else if (strncmp(mode, "PID", 3)) {
        PIDexe(param);
      }
      state = 0;
      break;
    default:
      state = 0;
      break;
  }
}


