#include <Wire.h>
#include <avr/wdt.h>
char serialbuf[64];
int sendIndex = 0;

void setup() {
  wdt_disable();
  wdt_enable(WDTO_4S);
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!Serial){
    return ;
  }
  Serial.println("slave reboot");
  Wire.begin(8);
  Wire.onRequest(requestEvent);
  sendIndex = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available() > 0){
    int len = Serial.read();
    printf("Serial received:%c",len);
    Wire.write(len);
  }
}

void requestEvent(){
  sendIndex++;
  Serial.print("i received:");
  Serial.println(Wire.read());
  if(sendIndex==1)
  {
    Wire.write('a');
    delayMicroseconds(500);
  }
  else if(sendIndex==2)
  {
    Wire.write('A');
    delayMicroseconds(500);
  }
  else if(sendIndex==3)
  {
    Wire.write('0');
    delayMicroseconds(500);
  }
  else if(sendIndex==4)
  {
    Wire.write(' ');  
    delayMicroseconds(500);
  }
  else if(sendIndex==5)
  {
    Wire.write('#'); 
    delayMicroseconds(500);
  }
  else if(sendIndex==6)
  {
    Wire.write('\0');  
    delayMicroseconds(500);
  }
  else if(sendIndex==7)
  {
    Wire.write('\n');
    delayMicroseconds(500);
  }
  else if(sendIndex==8)
  {
    Wire.write("He5#0");
    wdt_reset();
    delayMicroseconds(5000000);
  }
}
