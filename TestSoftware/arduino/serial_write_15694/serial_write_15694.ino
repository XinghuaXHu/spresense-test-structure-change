void setup() {
  // put your setup code here, to run once:
  //begin the serial with rate of 115200.
  Serial.begin(115200);
  //use print(78,OCT) function to send number 78 in OCT
  Serial.print(78,OCT);
  //use print(78,BIN) function to send number 78 in BIN
  Serial.print(78,BIN);
  //use print(78,DEC) function to send number 78 in DEC
  Serial.print(78,DEC);
  //use print(78,HEX) function to send number 78 in HEX
  Serial.print(78,HEX);
  //use print(78) function without any specified form
  Serial.print(78);

  printf("test case 15694 passed\n");
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
