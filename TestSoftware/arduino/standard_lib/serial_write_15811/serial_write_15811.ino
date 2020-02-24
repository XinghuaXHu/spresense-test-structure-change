int bundrate = 301;
//int bundrate = 950;
//int bundrate = 115219;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(bundrate);
  if(!Serial){
    return ;
  }

  printf("test start\r\n");

  Serial.write("abcdefg");

  printf("\r\ntest end\r\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
