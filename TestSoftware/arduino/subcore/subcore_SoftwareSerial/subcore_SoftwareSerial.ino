#include <MP.h>
#if (SUBCORE != 1)
#error "Core selection is wrong"
#endif

#include <SoftwareSerial.h>
#define SerialRX 2
#define SerialTX 3
SoftwareSerial mySoftSerial(SerialRX, SerialTX); // RX, TX

void setup()
{
  // Start the built-in serial port, probably to Serial Monitor
  int bundrate = 115200;
  mySoftSerial.begin(bundrate);
  Serial.begin(bundrate);
  if(!Serial){
    return ;
  }
  Serial2.begin(bundrate);
  if(!Serial){
    Serial2 ;
  }

  int ret;
  ret = MP.begin();
  MPLog("Start subcore1=%d\n",ret);

  MP.EnableConsole();
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(1000);
  while (Serial.available() > 0) {
      char inByte = Serial.read();
      Serial2.write(inByte);
  }
  delay(1000);
  bool firstCount = true;
  while (mySoftSerial.available() > 0) {
      char inByte = mySoftSerial.read();
      mySoftSerial.write(inByte);
      if(firstCount) 
        Serial.print("mySoftSerial read =");
      firstCount = false;
      if(mySoftSerial.available()==0)
        Serial.println(inByte);
      else
        Serial.print(inByte);
  }
  delay(1000);
  firstCount = true;
  while (Serial2.available() > 0) {
      char outByte = Serial2.read();
      if(firstCount) 
        Serial.print("mySoftSerial write=");
      firstCount = false;
      if(Serial2.available()==0)
        Serial.println(outByte);
      else
        Serial.print(outByte);
  }

}
