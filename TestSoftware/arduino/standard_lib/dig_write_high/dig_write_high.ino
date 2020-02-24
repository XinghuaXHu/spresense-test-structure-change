void setup() {
  Serial.begin(115200);
  while(!Serial){ 
  }
  // put your setup code here, to run once:
  for(int i=0;i<=PIN_D28;i++){
    pinMode(i,OUTPUT);
    digitalWrite(i,HIGH);
  }
  Serial.println("wrote D00-D28 pins HIGH");
}

void loop() {
  // put your main code here, to run repeatedly:
}
