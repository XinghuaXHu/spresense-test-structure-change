#include <MP.h>
#include "subcore_cmd.h"
#ifdef SUBCORE
#error "Core selection is wrong"
#endif

void setup() {
  // put your setup code here, to run once:
  // initialize serial communication at 9600 bits per second:
  int ret;

  Serial.begin(115200);

  Serial.println("Start Main core!");
   
  ret = MP.begin(1);
  Serial.print("Begin subcore1=");
  Serial.println(ret);

  pinMode(PIN_D00, OUTPUT);

  digitalWrite(PIN_D00,LOW);
   

  // make the pushbutton's pin an input:
}

void loop() {
  // put your main code here, to run repeatedly:
  int ret;
  int8_t    msgid;

  msgid = 100;

  ret = MP.Send(msgid,  CMD_START_PULSEIN, 1);
  delay(100);
  digitalWrite(PIN_D00,HIGH);
  delay(1000);
  digitalWrite(PIN_D00,LOW);
  delay(1000);
   
}

