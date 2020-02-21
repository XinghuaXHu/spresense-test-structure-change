#include <MP.h>
#if (SUBCORE != 1)
#error "Core selection is wrong"
#endif

void setup() {
  int d;
  int ret;

  Serial.begin(115200);
  while(!Serial){}
  
  ret = MP.begin();
  MPLog("Start subcore1=%d\n",ret);


  d = PIN_D00;

  pinMode(PIN_D00, OUTPUT);
  
  // put your setup code here, to run once:
  MPLog("test tone 15783\n");
  MPLog("test pin[%d]\n",d);
  MPLog("tone set 31\n");
  tone(d,31);
#if 1 
  delay(5000);
  MPLog("tone set 10000\n");
  tone(d,10000);
  delay(5000);
  MPLog("tone set 65535\n");
  tone(d,65535);
  delay(5000);
  MPLog("tone set notone\n");
  noTone(d);
  delay(5000);
  MPLog("test end\n\n");
#endif

}

void loop() {
  // put your main code here, to run repeatedly:

}
