void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  if(!Serial){
    return ;
  }

  printf("test start\n");

  Serial.println('A');

  printf("test end\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
