unsigned char test_pin;
unsigned char out_pin;
volatile int count = 0;
unsigned char current_pin_no;

void CountInterrupt(){
  delayMicroseconds(500);
  count++;
  syslog(LOG_INFO, "Interrupt occur! Pin[%d], count %d\n", current_pin_no, count);
}
void InterruptTestHIGH(unsigned char t_pin,unsigned char o_pin){
  count = 0;
  test_pin   = t_pin;
  out_pin = o_pin;
  current_pin_no = test_pin;
  
  printf("------Start Test pin[%d] Mode HIGH------\n",test_pin);
  pinMode(out_pin,OUTPUT);
  pinMode(test_pin,INPUT_PULLUP);

  //preconditions
  digitalWrite(out_pin,LOW);
  delay(1000);
  //1st step
  attachInterrupt(test_pin,CountInterrupt,HIGH);
  delay(1000);
  
  //2st step
  printf("to High\n");
  digitalWrite(out_pin,HIGH);
  delay(3000);
  //3st step
  printf("to Low\n");
  digitalWrite(out_pin,LOW);
  detachInterrupt(test_pin);
  delay(1000);
  
  //4st step
  printf("to High\n");
  digitalWrite(out_pin,HIGH);
  delay(3000);
  printf("to Low\n");
  digitalWrite(out_pin,LOW);

  //return result
  if(count == 5){
    printf("Test pin[%d] = OK\n", current_pin_no);
  }
  else{
    printf("Test pin[%d] = NG!\n", current_pin_no);
  }
}
void InterruptTestLOW(unsigned char t_pin,unsigned char o_pin){
  count = 0;
  test_pin   = t_pin;
  out_pin = o_pin;
  current_pin_no = test_pin;
  
  printf("------Start Test pin[%d] Mode LOW------\n",test_pin);
  pinMode(out_pin,OUTPUT);
  //preconditions
  digitalWrite(out_pin,HIGH);
  delay(1000);
  //1st step
  attachInterrupt(test_pin,CountInterrupt,LOW);
  delay(1000);

  //2st step
  printf("to LOW\n");
  digitalWrite(out_pin,LOW);
  delay(3000);
  //3st step
  printf("to HIGH\n");
  digitalWrite(out_pin,HIGH);
  detachInterrupt(test_pin);
  delay(1000);

  //4st step
  printf("to LOW\n");
  digitalWrite(out_pin,LOW);
  delay(3000);
  printf("to HIGH\n");
  digitalWrite(out_pin,HIGH);
 
  //return result
  if(count == 5){
    printf("Test pin[%d] = OK\n", current_pin_no);
  }
  else{
    printf("Test pin[%d] = NG!\n", current_pin_no);
  }
}
void InterruptTestRISING(unsigned char t_pin,unsigned char o_pin){
  count = 0;
  test_pin   = t_pin;
  out_pin = o_pin;
  current_pin_no = test_pin;
  
  printf("------Start Test pin[%d] Mode RISING------\n",test_pin);
  pinMode(out_pin,OUTPUT);
  //pinMode(test_pin,INPUT_PULLUP);

  //preconditions
  digitalWrite(out_pin,LOW);
  delay(1000);
  //1st step
  attachInterrupt(test_pin,CountInterrupt,RISING);
  delay(1000);
  
  //2st step
  printf("to High\n");
  digitalWrite(out_pin,HIGH);
  delay(3000);
  //3st step
  printf("to Low\n");
  digitalWrite(out_pin,LOW);
  detachInterrupt(test_pin);
  delay(1000);
  
  //4st step
  printf("to High\n");
  digitalWrite(out_pin,HIGH);
  delay(3000);
  printf("to Low\n");
  digitalWrite(out_pin,LOW);
 
  //return result
  if(count == 1){
    printf("Test pin[%d] = OK\n", current_pin_no);
  }
  else{
    printf("Test pin[%d] = NG!\n", current_pin_no);
  }
}
void InterruptTestFALLING(unsigned char t_pin,unsigned char o_pin){
  count = 0;
  test_pin   = t_pin;
  out_pin = o_pin;
  current_pin_no = test_pin;
  
  printf("------Start Test pin[%d] Mode FALLING------\n",test_pin);
  pinMode(out_pin,OUTPUT);
  //pinMode(test_pin,INPUT_PULLUP);

  //preconditions
  digitalWrite(out_pin,HIGH);
  delay(1000);
  //1st step
  attachInterrupt(test_pin,CountInterrupt,FALLING);
  delay(1000);
  
  //2st step
  printf("to LOW\n");
  digitalWrite(out_pin,LOW);
  delay(3000);
  //3st step
  printf("to HIGH\n");
  digitalWrite(out_pin,HIGH);
  detachInterrupt(test_pin);
  delay(1000);
  
  //4st step
  printf("to LOW\n");
  digitalWrite(out_pin,LOW);
  delay(3000);
  printf("to HIGH\n");
  digitalWrite(out_pin,HIGH);
 
  //return result
  if(count == 1){
    printf("Test pin[%d] = OK\n", current_pin_no);
  }
  else{
    printf("Test pin[%d] = NG!\n", current_pin_no);
  }
}
//CHANGE
void InterruptTestCHANGE(unsigned char t_pin,unsigned char o_pin){
  count = 0;
  test_pin   = t_pin;
  out_pin = o_pin;
  current_pin_no = test_pin;
  
  printf("------Start Test pin[%d] Mode CHANGE------\n",test_pin);
  pinMode(out_pin,OUTPUT);
  //pinMode(test_pin,INPUT_PULLUP);

  //preconditions
  digitalWrite(out_pin,LOW);
  delay(1000);
  //1st step
  attachInterrupt(test_pin,CountInterrupt,CHANGE);
  delay(1000);
  
  //2st step
  printf("to High\n");
  digitalWrite(out_pin,HIGH);
  delay(3000);
  //3st step
  printf("to Low\n");
  digitalWrite(out_pin,LOW);
  detachInterrupt(test_pin);
  delay(1000);
  
  //4st step
  printf("to High\n");
  digitalWrite(out_pin,HIGH);
  delay(3000);
  printf("to Low\n");
  digitalWrite(out_pin,LOW);
 
  //return result
  if(count == 1){
    printf("Test pin[%d] = OK\n", current_pin_no);
  }
  else{
    printf("Test pin[%d] = NG!\n", current_pin_no);
  }
}
int TestInterrupt(){
  //RISING
  InterruptTestRISING(PIN_D00,PIN_D01);
  InterruptTestRISING(PIN_D01,PIN_D00);
  
  InterruptTestRISING(PIN_D02,PIN_D03);
  InterruptTestRISING(PIN_D03,PIN_D02);
  
  InterruptTestRISING(PIN_D04,PIN_D05);
  InterruptTestRISING(PIN_D05,PIN_D04);
  
  InterruptTestRISING(PIN_D06,PIN_D07);
  InterruptTestRISING(PIN_D07,PIN_D06);
/*  
  InterruptTestRISING(PIN_D08,PIN_D09);
  InterruptTestRISING(PIN_D09,PIN_D08);
  
  InterruptTestRISING(PIN_D10,PIN_D11);
  InterruptTestRISING(PIN_D11,PIN_D10);
  
  InterruptTestRISING(PIN_D12,PIN_D13);
  InterruptTestRISING(PIN_D13,PIN_D12);
  
  InterruptTestRISING(PIN_D14,PIN_D15);
  InterruptTestRISING(PIN_D15,PIN_D14);
  
  InterruptTestRISING(PIN_D16,PIN_D17);
  InterruptTestRISING(PIN_D17,PIN_D16);
  
  InterruptTestRISING(PIN_D18,PIN_D19);
  InterruptTestRISING(PIN_D19,PIN_D18);
  
  InterruptTestRISING(PIN_D20,PIN_D21);
  InterruptTestRISING(PIN_D21,PIN_D20);
  
  InterruptTestRISING(PIN_D22,PIN_D23);
  InterruptTestRISING(PIN_D23,PIN_D22);
  
  InterruptTestRISING(PIN_D24,PIN_D25);
  InterruptTestRISING(PIN_D25,PIN_D24);
  
  InterruptTestRISING(PIN_D26,PIN_D27);
  InterruptTestRISING(PIN_D27,PIN_D26);
  
  InterruptTestRISING(PIN_D28,PIN_D14);
  */
}
void setup() {
  // put your setup code here, to run once:
  //Serial.begin(115200);
  //TestInterrupt();

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

      InterruptTestRISING(PIN_Test,PIN_Out);
    }

   printf("Test Done\n");
}



void loop() {
  // put your main code here, to run repeatedly:
 
  
  
}
