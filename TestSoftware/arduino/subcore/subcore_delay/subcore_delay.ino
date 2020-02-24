#include <MP.h>
#if (SUBCORE != 1)
#error "Core selection is wrong"
#endif
unsigned long start_millis = 0;
unsigned long start_micros = 0;
unsigned long end_millis = 0;
unsigned long end_micros = 0;

void setup()
{
  int ret;
  
  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(115200);

  ret = MP.begin();
  MPLog("Start subcore1=%d\n",ret);
}

void loop()
{
  MPLog("delay 1000 ms\n");
  start_millis = millis();
  delay(1000);
  end_millis = millis();

  MPLog("delay 1000 ms\n");
  start_micros = micros();
  delay(1000);
  end_micros = micros();

  //Serial.print("delta_millis : ");
  //Serial.print(end_millis - start_millis);
  //Serial.print(", delta_micros : ");
  //Serial.println(end_micros - start_micros);
  MPLog("delta_millis : %d, delta_micros : %d\n", end_millis - start_millis, end_micros - start_micros);
  
  MPLog("delay 1000000 us\n");
  start_millis = millis();
  start_micros = micros();
  delayMicroseconds(1000000);
  end_millis = millis();
  end_micros = micros();
 
  //Serial.print("delta_millis : ");
  //Serial.print(end_millis - start_millis);
  //Serial.print(", delta_micros : ");
  //Serial.println(end_micros - start_micros);
  MPLog("delta_millis : %d, delta_micros : %d\n", end_millis - start_millis, end_micros - start_micros);

  delay(5000);
}
