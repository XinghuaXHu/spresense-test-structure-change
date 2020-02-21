unsigned char test_pin;
unsigned char out_pin;
volatile int count = 0;
unsigned char current_pin_no;

void CountInterrupt(){
  delayMicroseconds(500);
  count++;
  syslog(LOG_INFO, "Interrupt occur! Pin[%d], count %d\n", current_pin_no, count);
  detachInterrupt(current_pin_no);
}

void InterruptTestFALLING(unsigned char t_pin,unsigned char o_pin){
  count = 0;
  test_pin   = t_pin;
  out_pin = o_pin;
  current_pin_no = test_pin;
  
  printf("------Start Test pin[%d] Mode FALLING------\n",test_pin);
  pinMode(out_pin,OUTPUT);
  //pinMode(test_pin,INPUT_PULLUP);
  //detachInterrupt(test_pin);
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
  //detachInterrupt(test_pin);
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

int TestInterrupt(){
  //RISING
  InterruptTestFALLING(PIN_D00,PIN_D01);
  InterruptTestFALLING(PIN_D01,PIN_D00);
  
  InterruptTestFALLING(PIN_D02,PIN_D03);
  InterruptTestFALLING(PIN_D03,PIN_D02);
  
  InterruptTestFALLING(PIN_D04,PIN_D05);
  InterruptTestFALLING(PIN_D05,PIN_D04);
  
  InterruptTestFALLING(PIN_D06,PIN_D07);
  InterruptTestFALLING(PIN_D07,PIN_D06);
/*
  InterruptTestFALLING(PIN_D08,PIN_D09);
  InterruptTestFALLING(PIN_D09,PIN_D08);
  
  InterruptTestFALLING(PIN_D10,PIN_D11);
  InterruptTestFALLING(PIN_D11,PIN_D10);
  
  InterruptTestFALLING(PIN_D12,PIN_D13);
  InterruptTestFALLING(PIN_D13,PIN_D12);
  
  InterruptTestFALLING(PIN_D14,PIN_D15);
  InterruptTestFALLING(PIN_D15,PIN_D14);
  
  InterruptTestFALLING(PIN_D16,PIN_D17);
  InterruptTestFALLING(PIN_D17,PIN_D16);
  
  InterruptTestFALLING(PIN_D18,PIN_D19);
  InterruptTestFALLING(PIN_D19,PIN_D18);
  
  InterruptTestFALLING(PIN_D20,PIN_D21);
  InterruptTestFALLING(PIN_D21,PIN_D20);
  
  InterruptTestFALLING(PIN_D22,PIN_D23);
  InterruptTestFALLING(PIN_D23,PIN_D22);
  
  InterruptTestFALLING(PIN_D24,PIN_D25);
  InterruptTestFALLING(PIN_D25,PIN_D24);
  
  InterruptTestFALLING(PIN_D26,PIN_D27);
  InterruptTestFALLING(PIN_D27,PIN_D26);
  
  InterruptTestFALLING(PIN_D28,PIN_D14);
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

      InterruptTestFALLING(PIN_Test,PIN_Out);
    }

   printf("Test Done\n");
}



void loop() {
  // put your main code here, to run repeatedly:
 
  
  
}
