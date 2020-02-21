char buf[16];

void test15773() {
  Serial.println("test analogWrite 15773");
  Serial.println("set analogWrite 0");
  analogWrite(3,0);
  analogWrite(5,0);
  analogWrite(6,0);
  analogWrite(9,0);
  sleep(10);
  Serial.println("set analogWrite 127");
  analogWrite(3,127);
  analogWrite(5,127);
  analogWrite(6,127);
  analogWrite(9,127);
  sleep(10);
  Serial.println("set analogWrite 255");
  analogWrite(3,255);
  analogWrite(5,255);
  analogWrite(6,255);
  analogWrite(9,255);
  sleep(10);
  Serial.println("test end");
}

void test15774() {
  Serial.println("test analogWrite 15774");
  analogWrite(3,0);
  analogWrite(5,64);
  analogWrite(6,127);
  analogWrite(9,255);
  Serial.println("test end");  
}

void setup() {
  memset(buf,0,16);
  Serial.begin(115200);
  Serial.println("Input \"[TestID];\"");
}

void loop() {
  if(Serial.available() <= 0){
    return;
  }
  String s = Serial.readStringUntil(';');
  if(s.compareTo("15773") == 0) {
    test15773();
  } else if(s.compareTo("15774") == 0) {
    test15774();
  } else {
    Serial.println("Unknown TestID");
  }
  Serial.println("Input \"[TestID];\"");
}
