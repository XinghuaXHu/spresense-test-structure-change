#include <MP.h>
#ifdef SUBCORE
#error "Core selection is wrong"
#endif

static const uint8_t s_out_pin = PIN_D00;

void setup() {
  // put your setup code here, to run once:
  // initialize serial communication at 9600 bits per second:
  int ret;

  Serial.begin(115200);

 MPLog("Start Main core!\n");
   
  ret = MP.begin(1);
  MPLog("Begin subcore1=%d\n",ret);

  ret = MP.begin(2);
  MPLog("Begin subcore2=%d\n",ret);

  pinMode(s_out_pin, OUTPUT);

  digitalWrite(s_out_pin,HIGH);
   

  // make the pushbutton's pin an input:
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  digitalWrite(s_out_pin,LOW);
  delay(1000);
  digitalWrite(s_out_pin,HIGH);
  //delay(1000);
   
}
