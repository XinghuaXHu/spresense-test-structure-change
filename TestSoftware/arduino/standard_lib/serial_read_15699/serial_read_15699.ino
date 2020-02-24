char buffer[16];
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!Serial){
    return ;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
//  memset(buffer,0,16);
//  if(Serial.available() > 0){
//    int len = Serial.readBytes(buffer,3);
//    printf("I recv len:%d,buff:%s\n",len,buffer);
//  }
  memset(buffer,0,16);
  if(Serial.available() > 0){
    int len = Serial.readBytes(buffer,3);
    printf("I recv len:%d,buff:%s\n",len,buffer);
  }
}
