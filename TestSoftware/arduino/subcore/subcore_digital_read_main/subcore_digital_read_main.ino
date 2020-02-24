#include <MP.h>
#ifdef SUBCORE
#error "Core selection is wrong"
#endif

void setup() {
  int ret;
  
  // put your setup code here, to run once:
  Serial.begin(115200);

  while(!Serial){}

  pinMode(PIN_D00, OUTPUT);

  MPLog("Start Main core!\n");
   
  ret = MP.begin(1);
  MPLog("Begin subcore1=%d\n",ret);

  digitalWrite(PIN_D00, LOW);

  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(PIN_D00, HIGH);
  delay(1000);
  digitalWrite(PIN_D00, LOW);
  delay(1000);

}
