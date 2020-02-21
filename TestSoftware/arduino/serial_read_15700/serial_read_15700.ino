char buffer[32];
int len = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);


  printf("test start\n");
  memset(buffer,0,32);
  
  /*
  memset(buffer,0,32);
  len = Serial.readBytesUntil('k',buffer,3);
  printf("readBytesUntil len:%d,buffer:%s\n",len,buffer);
  */
  /*
  memset(buffer,0,32);
  len = Serial.readBytesUntil('F',buffer,3);
  printf("readBytesUntil len:%d,buffer:%s\n",len,buffer);
  */

  /*
  memset(buffer,0,32);
  len = Serial.readBytesUntil('F',buffer,10);
  printf("readBytesUntil len:%d,buffer:%s\n",len,buffer);
  memset(buffer,0,32);
  */
  
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  if(Serial.available() > 0){
    len = Serial.readBytesUntil('k',buffer,3);
    printf("readBytesUntil len: %d,buffer: %s\n",len,buffer);
    Serial.flush();
  }
  */
  if(Serial.available() > 0){
    len = Serial.readBytesUntil('K',buffer,3);
    printf("readBytesUntil len: %d,buffer: %s\n",len,buffer);
    Serial.flush();
  }
}
