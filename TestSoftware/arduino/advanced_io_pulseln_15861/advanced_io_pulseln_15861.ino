void testPulseLOWLevel(uint8_t test_pin,uint8_t out_pin){
  uint8_t o_pin = out_pin;
  uint8_t t_pin = test_pin;
  long pulseValue = 0;

  printf("------test pin[%d] start------\n",t_pin);
  pinMode(o_pin,OUTPUT);
  pinMode(t_pin,INPUT);

  //1step
  printf("pin[%d] = %d\n",o_pin,HIGH);
  digitalWrite(o_pin,HIGH);

  //2 step
  pulseIn(t_pin,LOW);
  
  //3 step
  printf("pin[%d] = %d\n",o_pin,LOW);
  digitalWrite(o_pin,LOW);
  delay(10);
  printf("pin[%d] = %d\n",o_pin,HIGH);
  digitalWrite(o_pin,HIGH);

  pulseValue = pulseIn(t_pin,LOW);
  printf("1st pulseIn value: %d\n",pulseValue);
  //4step
  pulseIn(t_pin,LOW);
  printf("pin[%d] = %d\n",o_pin,LOW);
  digitalWrite(o_pin,LOW);
  delay(3000);
  printf("pin[%d] = %d\n",o_pin,HIGH);
  digitalWrite(o_pin,HIGH);

  pulseValue = pulseIn(t_pin,LOW);
  printf("2st pulseIn value: %d\n",pulseValue);

  printf("------test pin[%d] end------\n",t_pin);
}

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(115200);

  int i = 0;
  char cmd[16]={0,};
  // put your setup code here, to run once:

  printf("Please Input Test PIN Count:\n");
  gets(cmd);
  int PIN_Count = atol(cmd);

  for(i = 0; i < PIN_Count; i++ ){
    
      // put your setup code here, to run once:
      printf("Please Input Test PIN ID:\n");
      gets(cmd);
      printf("Please Input Out PIN ID:\n");
      int PIN_Test = atol(cmd);
      gets(cmd);
      int PIN_Out = atol(cmd);

      printf("-------------test case start-------------\n");
      testPulseLOWLevel(PIN_Test,PIN_Out);
      printf("-------------test case end-------------\n");
    }

   printf("Test Done\n");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
