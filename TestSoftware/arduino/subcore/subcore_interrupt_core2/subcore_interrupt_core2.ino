#include <nuttx/arch.h>
#include <MP.h>
#if (SUBCORE != 2)
#error "Core selection is wrong"
#endif

#define printf MPLog

static const uint8_t s_in_pin = PIN_D01;
//static const uint8_t s_out_pin = PIN_D00;
static unsigned long s_high_cnt = 0;
static unsigned long s_low_cnt = 0;
volatile int g_dbg = 1;

void isr_high(void)
{
  detachInterrupt(s_in_pin);
  ++s_high_cnt;
  MPLog("core2 s_high_cnt = %lu\n", s_high_cnt);
  attachInterrupt(s_in_pin, isr_low, LOW);
}

void isr_low(void)
{
  detachInterrupt(s_in_pin);
  ++s_low_cnt;
  MPLog("core2 s_low_cnt = %lu\n", s_low_cnt);
  attachInterrupt(s_in_pin, isr_high, HIGH);
  noInterrupts();
}

void isr_change(void)
{
  assert(up_interrupt_context());
  MPLog("%d\n",digitalRead(s_in_pin));
  detachInterrupt(s_in_pin);
  attachInterrupt(s_in_pin, isr_change, CHANGE);
}

void reset(void)
{
  s_high_cnt = 0;
  s_low_cnt = 0;
}

void setup() {
  //while (g_dbg) {}
  // put your setup code here, to run once:
  int ret;
  
  ret = MP.begin();
  
  Serial.begin(115200);
//  pinMode(s_out_pin, OUTPUT);
  pinMode(s_in_pin, INPUT);

  assert(digitalRead(s_in_pin) == HIGH);

  attachInterrupt(s_in_pin, isr_low, LOW);
//  delay(1000);
//  digitalWrite(s_out_pin, LOW);

//  delay(1000);
//  digitalWrite(s_out_pin, HIGH);
  
  /*
  assert(s_high_cnt == 1);
  assert(s_low_cnt == 0);
  digitalWrite(s_out_pin, LOW);
  delay(5);
  assert(s_high_cnt == 1);
  assert(s_low_cnt == 1);
  detachInterrupt(digitalPinToInterrupt(s_in_pin));
  */
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  digitalWrite(s_out_pin, HIGH);
  delay(10);
  digitalWrite(s_out_pin, LOW);
  delay(10);
  */
}

