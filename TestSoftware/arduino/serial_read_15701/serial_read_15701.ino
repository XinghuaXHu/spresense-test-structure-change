char buf[64];
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!Serial){
    
  }
  printf("test start\n");
  
  //int len = Serial.readBytesUntil('2',buf,10);
  //int len = Serial.readBytesUntil('A',buf,10);
  //int len = Serial.readBytesUntil('#',buf,10);
  //int len = Serial.readBytesUntil('4',buf,10);
  //int len = Serial.readBytesUntil(' ',buf,10);
  int len = Serial.readBytesUntil('G',buf,10);
  printf("I receviced len:%d,buffer:%s",len,buf);

  printf("test end\n");
}

void loop() {
  // put your main code here, to run repeatedly:
 
}
