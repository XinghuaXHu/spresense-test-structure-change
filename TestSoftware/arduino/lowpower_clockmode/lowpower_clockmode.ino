#include <stdio.h>
#include <RTC.h>
#include <LowPower.h>
#include <GNSS.h>
#include <Audio.h>

#include <arch/board/board.h>
#include "lcd.h"
#include <Camera.h>

#define BAUDRATE                (115200)
#define TIME_HEADER 'T' // Header tag for serial time sync message
#define MAX_ARG_NUM 5
#define MAX_BUF_LEN 20
char buf[MAX_BUF_LEN];
String args[MAX_ARG_NUM];

bool gnss_app = false;
bool player_app = false;

static int parse_args(String string)
{
  int from = 0;
  int pos;
  int index = 0;

  while( (pos = string.indexOf(" ", from)) != -1 )
  {
    if( index < MAX_ARG_NUM )
    {
      args[index++] = string.substring(from, pos);
      from = pos+1;
    }
    else
    {
      return -1;
    }
  }
  args[index++] = string.substring(from); 
  return index;
}

void setup()
{
  Serial.begin(BAUDRATE);
  while (!Serial)
  {
    ; /* wait for serial port to connect. Needed for native USB port only */
  }
  Serial.println("Lowpower clock mode change for MV HV LV");
}

void printClockMode()
{
  clockmode_e mode = LowPower.getClockMode();
  Serial.print("clock mode: ");
  switch (mode)
  {
  case CLOCK_MODE_156MHz:
    Serial.println("156MHz");
    break;
  case CLOCK_MODE_32MHz:
    Serial.println("32MHz");
    break;
  case CLOCK_MODE_8MHz:
    Serial.println("8MHz");
    break;
  }
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
      {
         printClockMode();
      }
    }
    else if(args[0] == "gnss" )
    {
      gnss_app = true;
      gnss_setup();
      Serial.println("gnss_setup()");
    }
    else if(args[0] == "dnnrt" )
    {
      dnnrt_setup();
      Serial.println("dnnrt_setup()");
    }
    else if(args[0] == "player" )
    {
      player_app = true;
      player_setup();
      Serial.println("player_setup()");
    }
    else if(args[0] == "camera" )
    {
      Serial.println("Start Camera test");
      init_lcd();
      Serial.println("Run: video_capture_test");
      video_capture_test();
    }
  }
  return 1;
}

void loop()
{ 
  if(gnss_app)
    gnss_loop();
  else if(player_app)
    player_loop();
  else
    wait_command_input();
}
