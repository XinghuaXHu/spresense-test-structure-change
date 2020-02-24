void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!Serial){
    return ;
  }
  printf("test start\n");
  printf("Write buffer size:%d\n",Serial.availableForWrite());

  int len = Serial.write("abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz012345678901234567890123456789012345678abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz012345678901234567890123456789012345678abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz012345678901234567890123456789012345678") ;
  printf("\nwrite data len:%d",len);
  printf("test end\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
