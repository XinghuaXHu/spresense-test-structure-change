unsigned long start_millis = 0;
unsigned long start_micros = 0;
unsigned long end_millis = 0;
unsigned long end_micros = 0;

void setup() {
  Serial.begin(115200);
  
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
}

void loop() {
  
  //ledOn(PIN_LED0);
#if 1
  Serial.println("delay 1000 ms");
  start_millis = millis();
  start_micros = micros();
  delay(1000);
  end_millis = millis();
  end_micros = micros();
#else
  printf("delay 1000 ms\n");
  start_millis = millis();
  delay(1000);
  end_millis = millis();

  printf("delay 1000 ms\n");
  start_micros = micros();
  delay(1000);
  end_micros = micros();
#endif
  Serial.print("delta_millis : ");
  Serial.print(end_millis - start_millis);
  Serial.print(", delta_micros : ");
  Serial.println(end_micros - start_micros);
  //printf("delta_millis : %d, delta_micros : %d\n", end_millis - start_millis, end_micros - start_micros);
    
  //ledOff(PIN_LED0);
  
  Serial.println("delay 1000000 us");
  start_millis = millis();
  start_micros = micros();
  delayMicroseconds(1000000);
  end_millis = millis();
  end_micros = micros();
  //printf("delta_millis : %d, delta_micros : %d\n", end_millis - start_millis, end_micros - start_micros);
  Serial.print("delta_millis : ");
  Serial.print(end_millis - start_millis);
  Serial.print(", delta_micros : ");
  Serial.println(end_micros - start_micros);

}
