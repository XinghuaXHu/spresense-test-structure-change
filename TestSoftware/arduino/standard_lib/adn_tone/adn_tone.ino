char buf[16];

void test15783(int pin_no) {
  int d = PIN_D00 + pin_no;
  printf("test tone 15783\n");
  printf("test pin[%d]\n",d);
  printf("tone set 31\n");
  tone(d,31);
  sleep(5);
  printf("tone set 10000\n");
  tone(d,10000);
  sleep(5);
  printf("tone set 65535\n");
  tone(d,65535);
  sleep(5);
  printf("tone set notone\n");
  noTone(d);
  sleep(5);
  printf("test end\n\n");
}

void test15784(int pin_no) {
  int d = PIN_D00 + pin_no;
  int next = (d == PIN_D28) ?PIN_D00 :d+1;
  printf("test tone 15784\n");
  printf("test pin[%d] pin[%d]\n",d,next);
  printf("tone set 10000\n");
  tone(d,10000);
  tone(next,10000);
  sleep(5);
  printf("tone set notone\n");
  noTone(d);
  noTone(next);
  printf("test end\n\n");
}

void test15786(int pin_no) {
  int d = PIN_D00 + pin_no;
  printf("test tone 15786\n");
  printf("test pin[%d]\n",d);
  printf("tone set 31\n");
  tone(d,31,4000);
  delay(5000);
  printf("tone duration completed\n");
  delay(5000);
  noTone(d);
  printf("test end\n\n");
}

void test15788(int pin_no) {
  int d = PIN_D00 + pin_no;
  printf("test tone 15788\n");
  printf("test pin[%d]\n",d);
  printf("tone set 31\n");
  tone(d,31,10000);
  sleep(5);
  printf("tone set notone\n");
  noTone(d);
  sleep(5);
  printf("test end\n\n");
}

void setup() {
  memset(buf,0,16);
  Serial.begin(115200);
}

void loop() {
  while(Serial.available() > 0) {
    Serial.read();
  }

  Serial.println("Input \"[TestID];\"");
  while(Serial.available() <= 0){}
  String t = Serial.readStringUntil(';');
  t.trim();  

  Serial.println("Input \"[PinNO];\"");
  while(Serial.available() <= 0){}
  String p = Serial.readStringUntil(';');
  p.trim();

  if(t.compareTo("15783") == 0) {
    test15783(p.toInt());
  } else if(t.compareTo("15784") == 0) {
    test15784(p.toInt());
  } else if(t.compareTo("15786") == 0) {
    test15786(p.toInt());
  } else if(t.compareTo("15788") == 0) {
    test15788(p.toInt());
  } else {
    Serial.println("Unknown TestID");
  }
}
