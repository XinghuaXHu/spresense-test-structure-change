/*
* Test sketch for
* spresense-16373
* spresense-16374
* spresense-16375
* spresense-16376
* spresense-16377
* spresense-16378
* spresense-16379
* spresense-16380
* spresense-16381
* spresense-16382
* spresense-16383
* spresense-16410
* spresense-16411
* spresense-16412
*/


#include <stdio.h>
#include <Watchdog.h>
#include <LowPower.h>
#include <RTC.h>


#define BAUDRATE                (115200)
#define MAX_ARG_NUM 5
#define MAX_BUF_LEN 20
char buf[MAX_BUF_LEN];
String args[MAX_ARG_NUM];

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

void setup() {
  Serial.begin(BAUDRATE);
  while (!Serial)
    {
      ; /* wait for serial port to connect. Needed for native USB port only */
    }
  Serial.println("Start Watchdog test");
}


int wait_command_input()
{
  int i;
  int arg_num = 0;
  
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
    if ( args[0] == "wd_begin")
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
   
