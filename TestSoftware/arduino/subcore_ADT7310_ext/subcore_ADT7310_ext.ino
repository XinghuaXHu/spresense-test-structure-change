#include <SPI.h>
#include <MP.h>
#if (SUBCORE != 1)
#error "Core selection is wrong"
#endif

/*
 * ADT7310での温度データ取得
 * ADT7310 - Arduino
 *  CS  - Pin10 SS(CS)
 *  SDI - Pin11 MOSI
 *  SDO - Pin12 MISO
 *  SCL - Pin13 SCK
 *  VDD - 5V
 *  GND - GND
 */
 
//int SSPIn = 10; // Pin10をSS
int SSPin = 9; // Pin9をSS
  
// 初期化
void setup(void) {

  int ret;

  Serial.begin(115200);

  ret = MP.begin();
  delay(1000);
  Serial.print("Start Subcore1=");
  Serial.println(ret);
  
  pinMode(SSPin, OUTPUT);


  Serial.print("ADT7310Test\n");
    
  SPI.setBitOrder(MSBFIRST);  //最上位ビット(MSB)から送信
  //SPI.setClockDivider(SPI_CLOCK_DIV128); //通信速度を遅く設定
// SPI.setClockDivider(SPI_CLOCK_DIV8); //通信速度を遅く設定
  SPI.setDataMode(SPI_MODE0); //CPOL(クロック極性):0 CPHA(クロックフェーズ):0
 
  SPI.begin();
  
  SetSSPin(LOW);
 
  //ソフトウェアリセット
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
     
  SPI.transfer(0x54);    // 測定開始（continuously read）
  delay(500);            // 仕様上は、240msで開始
}
  
// メインループ
void loop(void) {
  uint16_t uiVal;
  float fVal;
  int iVal;
  
  uiVal = (uint16_t)SPI.transfer(0) << 8;  // AD変換値 上位
  uiVal |= SPI.transfer(0);                // AD変換値 下位
 
  uiVal >>= 3;                          // シフトで13bit化   
  //uiVal = 0x1fff; //確認テスト用
  
  if(uiVal & 0x1000) {               // 13ビットで符号判定
    iVal = uiVal - 0x2000;         // マイナスの時 (10進数で8192)
  }
  else{
    iVal = uiVal;                 //プラスの時
  }
   
  fVal = (float)iVal / 16.0;           // 温度換算(摂氏)
  Serial.println(fVal, 4);             // シリアル送信
   
  delay(1000);          //例として5秒待つ
}
 
/*
 * SSピンの設定
 * Lowでマスタからの通信が有効（セレクト）
 * (Highではマスタから通信出来ない(無視される）セレクト解除の状態)
 */
void SetSSPin(int val)
{ 
    digitalWrite(SSPin, val); 
}


