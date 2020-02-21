#include <MP.h>
#include <MP.h>
#if (SUBCORE != 1)
#error "Core selection is wrong"
#endif

void setup() {
  int ret;
  
  // put your setup code here, to run once:
  Serial.begin(115200);

  while(!Serial){}

  ret = MP.begin();


  pinMode(PIN_D00, OUTPUT);

  digitalWrite(PIN_D00, LOW);

  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(PIN_D00, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_D00, LOW);
  delayMicroseconds(10);

}
