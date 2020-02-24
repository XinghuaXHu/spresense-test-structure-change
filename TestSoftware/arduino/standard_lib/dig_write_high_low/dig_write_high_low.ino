void setup() {
  Serial.begin(115200);
  while(!Serial){ 
  }
  // put your setup code here, to run once:
  for(int i=0;i<=PIN_D28;i++){
    pinMode(i,OUTPUT);
    if(i%2 == 0){
      digitalWrite(i,HIGH);
    }else{
      digitalWrite(i,LOW);
    }
  }
  Serial.println("wrote D00-D28 pins High,Low...");
}

void loop() {
  // put your main code here, to run repeatedly:
}
