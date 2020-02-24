#include <MP.h>
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

  

  // make the pushbutton's pin an input:
}

void loop() {
  // put your main code here, to run repeatedly:
   
}

