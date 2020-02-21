#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  Wire.begin(8);  
  Wire.onReceive(receiveEvent);
  Serial.begin(115200);
  Serial.println("test wire start");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}

void receiveEvent(int howMany){
  while(1 < Wire.available()){
    char c = Wire.read();
    Serial.print("I receive:");
    Serial.print(c, DEC);
    Serial.print("------");
    Serial.write(c);
    Serial.println(" ");
  }
  int x = Wire.read();
  Serial.print("I receive:");
  Serial.print(x); 
  Serial.print("------");
  Serial.write(x);
  Serial.println(" ");
}
