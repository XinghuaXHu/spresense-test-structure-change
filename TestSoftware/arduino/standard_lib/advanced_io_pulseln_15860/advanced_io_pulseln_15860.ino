#include <RTC.h>
#include <arch/chip/pm.h>
uint8_t t_pin;
uint8_t o_pin;
int cnt = 0;

void alarmExpired(void)
{
  if ((cnt % 3) == 0)
  {
    printf("pin[%d] = %d\n",o_pin,HIGH);
    digitalWrite(o_pin,HIGH);
    usleep(10);
    digitalWrite(o_pin,LOW);
    printf("pin[%d] = %d\n",o_pin,LOW);  
  }
  else if ((cnt % 3) == 1)
  {
    printf("pin[%d] = %d\n",o_pin,HIGH);
    digitalWrite(o_pin,HIGH); 
    RtcTime now = RTC.getTime();
    now += 180;
    RTC.setAlarm(now);
  }
  else
  {
    digitalWrite(o_pin,LOW);
    printf("pin[%d] = %d\n",o_pin,LOW);  
  }
  cnt++;
}

void testPulseHighLevel(uint8_t test_pin,uint8_t out_pin){
  cnt = 0;
  o_pin = out_pin;
  t_pin = test_pin;
  long pulseValue = 0;

  printf("------test pin[%d] start------\n",t_pin);
  pinMode(o_pin,OUTPUT);
  pinMode(t_pin,INPUT);

  printf("pin[%d] = %d\n",o_pin,LOW);
  digitalWrite(o_pin,LOW);

  RtcTime now = RTC.getTime();
  now += 1;
  RTC.setAlarm(now);
  
  printf("1st pulseIn value\n");
  pulseValue = pulseIn(t_pin,HIGH);
  printf("1st pulseIn value: %d\n",pulseValue);
  
  now = RTC.getTime();
  now += 1;
  RTC.setAlarm(now);
  
  printf("2nd pulseIn value\n");
  pulseValue = pulseIn(t_pin,HIGH, 181*1000000ul);
  printf("2nd pulseIn value: %d\n",pulseValue);

  printf("------test pin[%d] end------\n",t_pin);
}

void setup() {
  // put your setup code here, to run once:
  printf("Please Input PIN ID:\n");
  char cmd[16]={0,};
  gets(cmd);
  printf("Please Input PIN ID:\n");
  int PIN_Test = atol(cmd);
  gets(cmd);
  int PIN_Out = atol(cmd);
  
  // Initialize RTC at first
  RTC.begin();

  // Set the RTC alarm handler
  RTC.attachAlarm(alarmExpired);

  printf("-------------test case start-------------\n");
  testPulseHighLevel(_DIGITAL_PIN(PIN_Test),_DIGITAL_PIN(PIN_Out));

  printf("-------------test case end-------------\n");
  delay(1000);
  up_pm_reboot();
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
