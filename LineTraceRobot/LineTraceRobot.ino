#include <SoftwareSerial.h>

//Bluetoothモジュール
const int BtRx = 13; //受信用ピン
const int BtTx = 12; //送信用ピン
SoftwareSerial btSerial(BtRx,BtTx); //Bluetooth用シリアル通信設定

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
double parm[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

String text = "";
char mode[7] = "";

//文字列を送信する関数
void TransString(String txt){
  int i = 0;
  for(i = 0; i < txt.length(); i++){
    btSerial.write(txt.charAt(i));
  }
}

//文字列を受信する関数
String ReceiveString(){
  String txt = "";

  return txt;
}

//スイッチが押されたか確認する関数
int SwitchCheck(){
  if(digitalRead(Switch)==LOW){
    delay(50); //チャタリング防止
    if(digitalRead(Switch)==LOW){
      return 1;
    }
  }
  return 0;
}

//フォトリフレクタの値が閾値を超えているか確認する関数
//閾値より大きい=黒、小さい=白
int Sensor(int pin){
  int v = analogRead(pin);
  if(v>thresh){
    return 1;
  }else{
    return 0;
  }
}

//ONOFF制御実行関数
void ONOFFexe(){
  
}

//PID制御実行関数
void PIDexe(){
  
}

void setup() {
  btSerial.begin(15200); //Bluetoothシリアル開始
  pinMode(Switch,INPUT_PULLUP); //プルアップ抵抗
  
  //モーターのピンを初期化
  pinMode(motorR1,OUTPUT);
  pinMode(motorR2,OUTPUT);
  pinMode(motorL1,OUTPUT);
  pinMode(motorL2,OUTPUT);
  digitalWrite(motorR1,LOW);
  digitalWrite(motorR2,LOW);
  digitalWrite(motorL1,LOW);
  digitalWrite(motorL2,LOW);
}

void loop() {
  switch(state){
    case 0://待機
      if(SwitchCheck()==1){
        delay(500);
        TransString(String("Ready"));
        state = 1;
      }
      break;
    case 1://受信待機
      if(SwitchCheck()==1){//エラー時はスイッチを押してリセット
        state = 0;
      }else if(btSerial.available()){//受信データ有り
        text = ReceiveString();
        char str[128] = "";
        strcpy(str,text.c_str());
        int i;

        state = 2;
      }
      break;
    case 2://実行
      delay(1000);
      if(mode){
        ONOFFexe();
      }else if(mode){
        PIDexe();
      }
      state = 0
      break;
    default:
      state = 0;
      break;
  }
}

