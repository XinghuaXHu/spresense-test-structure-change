#include <RTC.h>

#define BAUDRATE                (115200)
RtcTime compiledDateTime(__DATE__, __TIME__);
RtcTime now;

void printClock(RtcTime &rtc)
{
  printf("%04d/%02d/%02d %02d:%02d:%02d\n",
         rtc.year(), rtc.month(), rtc.day(),
         rtc.hour(), rtc.minute(), rtc.second());
}

void alarmExpired(void)
{
  static int cnt = 0;

  RtcTime now = RTC.getTime();

  // Set the RTC alarm every 5 seconds
  //RtcTime alm = now + 5;
  //RTC.setAlarm(alm);

  printClock(now);
}


void setup() {
  Serial.begin(BAUDRATE);
  while (!Serial)
  {
    ; /* wait for serial port to connect. Needed for native USB port only */
  }
  Serial.println("Start RTC test");

  
  RTC.begin();
  RTC.setTime(compiledDateTime);
  printClock(compiledDateTime);

  RTC.attachAlarm(alarmExpired);
  now = RTC.getTime();
  now += 10;
  RTC.setAlarm(now);
  Serial.println("set alarm 10 seconds");
}

void loop() {
  usleep(1000 * 1000 * 20);
  Serial.println("loop()");
  RTC.setTime(compiledDateTime);
  printClock(compiledDateTime);
  RTC.attachAlarm(alarmExpired);
  now = RTC.getTime();
  now += 15;
  RTC.setAlarm(now);
  usleep(1000 * 1000 * 20);
  RTC.end();
  usleep(1000 * 1000 * 20);
  Serial.println("RTC.end()");
}
