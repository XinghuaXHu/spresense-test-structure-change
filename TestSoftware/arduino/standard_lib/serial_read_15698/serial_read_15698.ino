char buf = 0;
void setup() {
  // put your setup code here, to run once:
  //begin the serial with rate of 115200.
  Serial.begin(115200);
  //Counterpart device sent 'a' to the Sparduino.
  //Sparduino uses read() function.
  //repeat the last 2, 3 Step actions except that the byte sent by counterpart device and is 'B', '6', '$', ' '.
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0){
    buf = Serial.read();
    Serial.print("i received:");
    Serial.println(buf);
  }
}
