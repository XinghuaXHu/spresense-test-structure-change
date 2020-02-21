#include <MP.h>
#if (SUBCORE != 1)
#error "Core selection is wrong"
#endif

#define PWM_PIN 6

void setup()
{
  int ret;
  
  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(115200);

  ret = MP.begin();
  Serial.print("Start subcore1=");
  Serial.println(ret);
  
}

void loop()
{
  int byteReceived;
  int byteSend;

  analogWrite(PWM_PIN, 128);
}
