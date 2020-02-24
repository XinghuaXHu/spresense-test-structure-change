#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(115200);
  if(!Serial){
    return ;
  }
  printf("test start\n");
  printf("Config adress:8,value len:5\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  Wire.requestFrom(8,1);
  while(Wire.available()){
    char c = Wire.read();
    Serial.print("I received:");
    Serial.println(c);
  }

  Wire.requestFrom(8,1);
  while(Wire.available()){
    char c = Wire.read();
    Serial.print("I received:");
    Serial.println(c);
  }

  Wire.requestFrom(8,1);
  while(Wire.available()){
    char c = Wire.read();
    Serial.print("I received:");
    Serial.println(c);
  }

  Wire.requestFrom(8,1);
  while(Wire.available()){
    char c = Wire.read();
    Serial.print("I received:");
    Serial.println(c);
  }

  Wire.requestFrom(8,1);
  while(Wire.available()){
    char c = Wire.read();
    Serial.print("I received:");
    Serial.println(c);
  }

  Wire.requestFrom(8,1);
  while(Wire.available()){
    char c = Wire.read();
    Serial.print("I received:");
    Serial.print("DEC=");
    Serial.println(c,DEC);
  }

  Wire.requestFrom(8,1);
  while(Wire.available()){
    char c = Wire.read();
    Serial.print("I received:");
    Serial.print("DEC=");
    Serial.println(c,DEC);
  }

  Wire.requestFrom(8,5);
  while(Wire.available()){
    char c = Wire.read();
    Serial.print("I received:");
    Serial.println(c);
  }
  delayMicroseconds(5000000);
}
