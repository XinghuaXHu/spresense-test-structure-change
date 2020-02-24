#include <stdio.h>
#include <RTC.h>
#include <LowPower.h>

#define BAUDRATE                (115200)
#define TIME_HEADER 'T' // Header tag for serial time sync message
#define MAX_ARG_NUM 5
#define MAX_BUF_LEN 20
char buf[MAX_BUF_LEN];
String args[MAX_ARG_NUM];

const uint8_t button2 = PIN_D02;

void pushed2()
{
  Serial.println("Pushed D02!");
}

static int parse_args(String string)
{
    int from = 0;
    int pos;
    int index = 0;

    while( (pos = string.indexOf(" ", from)) != -1 ){
        if( index < MAX_ARG_NUM ){
            args[index++] = string.substring(from, pos);
            from = pos+1;
        }
        else{
            return -1;
        }
    }
    
    args[index++] = string.substring(from); 
    
    return index;
}


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
}


int wait_command_input()
{
  int i;
  int arg_num = 0;
  RtcTime compiledDateTime(__DATE__, __TIME__);
  RtcTime now;
  
  if ( Serial.available() > 0 )
  {
    String str = Serial.readStringUntil('\r');
    arg_num = parse_args(str);
    if( arg_num == -1 )
    {
      Serial.println("error argument");
      return 0;
    }
    else
    {
      Serial.print("> ");
      for( i=0; i<arg_num; i++)
      {
        Serial.print(args[i]);
        Serial.print(" ");
      }
      Serial.println();
    }
    
    Serial.println();
    if ( args[0] == "rtc_begin")
    {     
      RTC.begin(); 
      Serial.println("RTC: begin()");
    }
    else if ( args[0] == "rtc_end" )
    {
      RTC.end();
      Serial.println("RTC: end()");
    }
    else if ( args[0] == "rtc_settime" )
    {
      RTC.setTime(compiledDateTime);
      printClock(compiledDateTime);
      Serial.println("RTC: settime()");
    }
    else if ( args[0] == "rtc_gettime" )
    {
      now = RTC.getTime();
      printClock(now);
      Serial.println("RTC: gettime()");
    }
    else if ( args[0] == "rtc_setalarm" )
    {      
      if( arg_num == 2 )
      {
        int arg1;
        args[1].toCharArray(buf, MAX_BUF_LEN);
        arg1 = atoi(buf);
        
        now = RTC.getTime();
        printClock(now);
        now += arg1;
        RTC.setAlarm(now);
        Serial.println("RTC: setAlarm()");
        Serial.print("alarm after ");
        Serial.print(arg1);
        Serial.println(" seconds");
      }
      else
      {
        Serial.println("Error invalid parameter");
      }
    }
    else if ( args[0] == "rtc_setalarmsec" )
    {      
      if( arg_num == 2 )
      {
        uint32_t arg1;
        args[1].toCharArray(buf, MAX_BUF_LEN);
        arg1 = atoi(buf);
        RTC.setAlarmSeconds(arg1);
        Serial.println("RTC: setAlarmSeconds()");
        now = RTC.getTime();
        printClock(now);
        Serial.print("alarm after ");
        Serial.print(arg1);
        Serial.println(" seconds");
      }
      else
      {
        Serial.println("Error invalid parameter");
      }
    }
    else if ( args[0] == "rtc_cancelalarm" )
    {
      RTC.cancelAlarm();
      Serial.println("RTC: cancelAlarm()");
    }
    else if ( args[0] == "rtc_attach" )
    {
      RTC.attachAlarm(alarmExpired);
      Serial.println("RTC: attachAlarm()");
    }
    else if ( args[0] == "rtc_detach" )
    {
      RTC.detachAlarm();
      Serial.println("RTC: detachAlarm()");
    }
    else if(args[0] == "sleep_deep")
    {
      if( arg_num == 2 )
      {
        LowPower.begin();
        int arg1;
        args[1].toCharArray(buf, MAX_BUF_LEN);
        arg1 = atoi(buf);
        Serial.println("Go to sleep");
        LowPower.deepSleep(arg1);
      }
      else
      {
        Serial.println("Error invalid parameter");
      }
    }
    else if(args[0] == "sleep_gpio")
    {
      LowPower.begin();
      pinMode(button2, INPUT_PULLUP);
      attachInterrupt(button2, pushed2, FALLING);
      delay(5000);
      LowPower.enableBootCause(button2);    
      Serial.println("Go to sleep");
      delay(1000);
      LowPower.coldSleep();
    }
    else
    {
     Serial.print("error [");
     Serial.print(str);
     Serial.println("]");
    }
  }
  return 1;
}

void loop() {
  wait_command_input();
}
