#include <MP.h>
#ifdef SUBCORE
#error "Core selection is wrong"
#endif

void setup() {
  // put your setup code here, to run once:
  // initialize serial communication at 9600 bits per second:
  int ret;

  MPLog("Start Main core!\n");
   
  ret = MP.begin(1);
  MPLog("Begin subcore1=%d\n",ret);
   
  Serial.begin(115200);
  // make the pushbutton's pin an input:
  pinMode(PIN_D01, INPUT);
  


}

void loop() {
  // put your main code here, to run repeatedly:
  int value;

  value = digitalRead(PIN_D01);

  MPLog("Maincore Value=%d\n",value);
  delay(100);

}
