void setup() {
  // put your setup code here, to run once:
  for(int i=0;i<=PIN_D28;i++){
    pinMode(i,INPUT_PULLUP);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0;i<=PIN_D28;i++){
    printf("pin[%d]=[%d]\n",i,digitalRead(i));
  }
  sleep(1);
}
