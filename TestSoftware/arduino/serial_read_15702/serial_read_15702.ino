int timeout = 5000;
//int timeout = 0;


char buf[36];
int old_time = 0;
int new_time = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Sparduino uses function setTimeout() with the parametor of 5000. 
  //"Ad4 y$v" are sended from another device.
  Serial.setTimeout(timeout);
  printf("test case 15702\n");
  
  //use function millis() to get the system clock  
  old_time = millis();
  printf("Time:%d\n",old_time);
  //use readBytesUntil() with parameter of terminator 'y' and length 10 to read bytes into buffer
  memset(buf,0,36);
  int resultlen = Serial.readBytesUntil('y',buf,10);
  //int resultlen = Serial.readBytesUntil('Z',buf,5);
  //int resultlen = Serial.readBytesUntil('Z',buf,15);
  //use function millis() to get the system clock  
  new_time = millis();
  printf("Time:%d\n",new_time);
  printf("use time:%6d, result char sum:%6d, buf:%s\n",new_time-old_time,resultlen,buf);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
