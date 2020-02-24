#include <stdio.h>
#include <RTC.h>
#include <LowPower.h>
#include <Watchdog.h>

#define BAUDRATE                (115200)
#define TIME_HEADER 'T' // Header tag for serial time sync message
#define MAX_ARG_NUM 5
#define MAX_BUF_LEN 20
char buf[MAX_BUF_LEN];
String args[MAX_ARG_NUM];

/* delay test function */
unsigned long start_millis = 0;
unsigned long start_micros = 0;
unsigned long end_millis = 0;
unsigned long end_micros = 0;
void delay_test_function() {
  
  printf("delay 1000 ms\n");
  start_millis = millis();
  delay(1000);
  end_millis = millis();

  printf("delay 1000 ms\n");
  start_micros = micros();
  delay(1000);
  end_micros = micros();

  Serial.print("delta_millis : ");
  Serial.print(end_millis - start_millis);
  Serial.print(", delta_micros : ");
  Serial.println(end_micros - start_micros);
  //printf("delta_millis : %d, delta_micros : %d\n", end_millis - start_millis, end_micros - start_micros);
  
  Serial.println("delay 1000000 us");
  start_millis = millis();
  start_micros = micros();
  delayMicroseconds(1000000);
  end_millis = millis();
  end_micros = micros();
 
  Serial.print("delta_millis : ");
  Serial.print(end_millis - start_millis);
  Serial.print(", delta_micros : ");
  Serial.println(end_micros - start_micros);
  //printf("delta_millis : %d, delta_micros : %d\n", end_millis - start_millis, end_micros - start_micros);
}


const char* boot_cause_strings[] = {
  "Power On Reset with Power Supplied",
  "System WDT expired or Self Reboot",
  "Chip WDT expired",
  "WKUPL signal detected in deep sleep",
  "WKUPS signal detected in deep sleep",
  "RTC Alarm expired in deep sleep",
  "USB Connected in deep sleep",
  "Others in deep sleep",
  "SCU Interrupt detected in cold sleep",
  "RTC Alarm0 expired in cold sleep",
  "RTC Alarm1 expired in cold sleep",
  "RTC Alarm2 expired in cold sleep",
  "RTC Alarm Error occurred in cold sleep",
  "Unknown(13)",
  "Unknown(14)",
  "Unknown(15)",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "GPIO detected in cold sleep",
  "SEN_INT signal detected in cold sleep",
  "PMIC signal detected in cold sleep",
  "USB Disconnected in cold sleep",
  "USB Connected in cold sleep",
  "Power On Reset",
};

void printBootCause(bootcause_e bc)
{
  Serial.println("--------------------------------------------------");
  Serial.print("Boot Cause: ");
  Serial.print(boot_cause_strings[bc]);
  Serial.println();
  Serial.println("--------------------------------------------------");
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

const uint8_t button2 = PIN_D02;

void pushed2()
{
  Serial.println("Pushed D02!");
}

void printClock(RtcTime &rtc)
{
  printf("%04d/%02d/%02d %02d:%02d:%02d\n",
         rtc.year(), rtc.month(), rtc.day(),
         rtc.hour(), rtc.minute(), rtc.second());
}
volatile int alarmInt = 0;
void alarmExpired(void)
{
  alarmInt = 1;
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
  Serial.println("Start WATCHDOG RTC STD lib test in low-power case");
}

void printClockMode()
{
  clockmode_e mode = LowPower.getClockMode();

  Serial.println("--------------------------------------------------");
  Serial.print("clock mode: ");
  switch (mode) {
    case CLOCK_MODE_156MHz: Serial.println("156MHz"); break;
    case CLOCK_MODE_32MHz:  Serial.println("32MHz"); break;
    case CLOCK_MODE_8MHz:   Serial.println("8MHz"); break;
  }
  Serial.println("--------------------------------------------------");
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
    if ( args[0] == "low")
    {
       if( arg_num == 2 && args[1] == "HV" )
      {
        LowPower.clockMode(CLOCK_MODE_156MHz);
        printClockMode();
      }
      else if( arg_num == 2 && args[1] == "MV" )
      {
        LowPower.clockMode(CLOCK_MODE_32MHz);
        printClockMode();
      }
      else if( arg_num == 2 && args[1] == "LV" )
      {
        LowPower.clockMode(CLOCK_MODE_8MHz);
        printClockMode();
      }
      else if( arg_num == 2 && args[1] == "begin" )
      {
        LowPower.begin();
        Serial.println("LowPower: begin()");
      }
      else if( arg_num == 2 && args[1] == "end" )
      {
        LowPower.end();
        Serial.println("LowPower: end()");
      }
      else
         printClockMode();
    }
    else if ( args[0] == "getboot")
    {     
      // Get the boot cause
      bootcause_e bc = LowPower.bootCause();
      if ((bc == POR_SUPPLY) || (bc == POR_NORMAL)) {
        Serial.println("Example for system reboot by watchdog");
      } else {
        Serial.println("rebooted by watchdog");
      }
      Serial.println("--------------------------------------------------");
      Serial.print("Boot Cause: ");
      Serial.print(boot_cause_strings[bc]);
      Serial.println();
      Serial.println("--------------------------------------------------");
    }
    else if ( args[0] == "delay_test")
    {
      delay_test_function();
    }
    else if ( args[0] == "wd_begin")
    {     
      Watchdog.begin(); 
      Serial.println("Watchdog: begin()");
    }
    else if ( args[0] == "wd_end" )
    {
      Watchdog.end();
      Serial.println("Watchdog: end()");
    }
    else if(args[0] == "wd_start")
    {
      if( arg_num == 2 )
      {
        uint32_t arg1;
        args[1].toCharArray(buf, MAX_BUF_LEN);
        arg1 = atoi(buf);
        Watchdog.start(arg1);
        Serial.println("Watchdog: start(ms-sec)");
        Serial.println( Watchdog.timeleft() ) ;
        Serial.println("ms left for watchdog bite");
      }
      else
      {
        Serial.println("Error invalid parameter");
      }
    }
    else if(args[0] == "wd_stop")
    {
      Watchdog.stop();
      Serial.println("Watchdog: stop()");
      Serial.println( Watchdog.timeleft() ) ;
      Serial.println("ms left for watchdog bite");
    }
    else if( args[0] == "wd_kick")
    {
      Watchdog.kick();
      Serial.println("Watchdog: kick()");
      Serial.println( Watchdog.timeleft() ) ;
      Serial.println("ms left for watchdog bite");
    }
    else if(args[0] == "wd_timeleft")
    { 
      Serial.println( Watchdog.timeleft() ) ;
      Serial.println("ms left for watchdog bite");
    }
    else if(args[0] == "sleep_cold")
    {
      if( arg_num == 2 )
      {
        LowPower.begin();
        int arg1;
        args[1].toCharArray(buf, MAX_BUF_LEN);
        arg1 = atoi(buf);
        Serial.println("Go to sleep");
        LowPower.coldSleep(arg1);
      }
      else
      {
        Serial.println("Error invalid parameter");
      }
    }
    else if ( args[0] == "rtc_begin")
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

void loop()
{  
  wait_command_input();
}
   
