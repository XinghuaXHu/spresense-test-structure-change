#include <MP.h>
#include "subcore_cmd.h"
#if (SUBCORE != 1)
#error "Core selection is wrong"
#endif

void setup() {
  // put your setup code here, to run once:
  int ret;
  
  ret = MP.begin();
  
  Serial.begin(115200);

  pinMode(PIN_D01, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int ret;
  int8_t    msgid;
  uint32_t  cmd;
  int duration;
  
  ret = MP.Recv(&msgid, &cmd);
  if(ret < 0){
    return;
  }
  
  if(cmd == CMD_START_PULSEIN){
    duration = pulseIn(PIN_D01, HIGH, 2000000);
    Serial.print("Duration=");
    Serial.println(duration);
  }
  
}
