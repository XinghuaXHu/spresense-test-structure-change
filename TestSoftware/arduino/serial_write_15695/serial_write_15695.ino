void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!Serial){
    return ;
  }

  printf("test start\n");

  Serial.print(1.23456,0);
  Serial.print(1.23456,1);
  Serial.print(1.23456,5);

  printf("test end\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
