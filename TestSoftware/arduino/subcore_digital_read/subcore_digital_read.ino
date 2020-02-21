#include <MP.h>
#if (SUBCORE != 1)
#error "Core selection is wrong"
#endif

void setup() {
  // put your setup code here, to run once:
  // initialize serial communication at 9600 bits per second:
  int ret;

   ret = MP.begin();
   
  Serial.begin(115200);
  // make the pushbutton's pin an input:
  pinMode(PIN_D01, INPUT);
  


}

void loop() {
  // put your main code here, to run repeatedly:
  int value;

  value = digitalRead(PIN_D01);

  MPLog("Subcore Value=%d\n",value);

  delay(100);

}
