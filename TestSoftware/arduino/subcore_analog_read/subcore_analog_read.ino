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

  MPLog("Start analog read test\n");
}

void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  // print out the value you read:
  MPLog("Voltage=%1.2f\n",voltage);
}
