void setup() {
  Serial.begin(115200);
  while(!Serial){ 
  }
  Serial.println("read D00-D28 pins");
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0;i<=PIN_D28;i++){
    pinMode(i,INPUT);
    printf("pin[%d] = [%d]\n",i,digitalRead(i));
  }
  Serial.println("delay 3sec");
  delay(3000);
}
