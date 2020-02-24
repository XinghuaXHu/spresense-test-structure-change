//int bundrate = 300;
//int bundrate = 600;
//int bundrate = 1200;
//int bundrate = 2400;
//int bundrate = 4800;
//int bundrate = 9600;
//int bundrate = 14400;
//int bundrate = 19200;
//int bundrate = 28800;
//int bundrate = 38400;
//int bundrate = 57600;
int bundrate = 115200;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(bundrate);
  if(!Serial){
    return ;
  }

  printf("test start\r\n");

  Serial.write('a');

  printf("\r\ntest end\r\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
