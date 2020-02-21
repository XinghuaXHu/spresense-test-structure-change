/*
 *  MessageHello.ino - MP Example to communicate message strings
 *  Copyright 2019 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <MP.h>
#include <stdio.h>
#include <LowPower.h>

#define MSGLEN      64
#define MY_MSGID    10
struct MyPacket {
  volatile int status; /* 0:ready, 1:busy */
  char message[MSGLEN];
};


#define BAUDRATE                (115200)
#define TIME_HEADER 'T' // Header tag for serial time sync message
#define MAX_ARG_NUM 5
#define MAX_BUF_LEN 20
char buf[MAX_BUF_LEN];
String args[MAX_ARG_NUM];

int subid;
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
    
    else if ( args[0] == "core1")
    {
      core_setup(1);
      Serial.println("core1_setup()");
    }
    else if(args[0] == "core2" )
    {
      core_setup(2);
      Serial.println("core2_setup()");
    }
    else if(args[0] == "core3" )
    {
      core_setup(3);
      Serial.println("core3_setup()");
    }
    else if(args[0] == "core4" )
    {
      core_setup(4);
      Serial.println("core4_setup()");
    }
    else if(args[0] == "core5" )
    {
      core_setup(5);
      Serial.println("core5_setup()");
    }

}
  return 1;
}

void setup()
{
  
  

  Serial.begin(115200);
  while (!Serial);
  sleep(3);
  Serial.println("Start subcore power test");
}

void core_setup (int subid)
{
  int ret = 0;
  ret = MP.begin(subid);
  if (ret < 0) {
      printf("MP.begin(%d) error = %d\n", subid, ret);
    }
  MP.RecvTimeout(MP_RECV_POLLING);
  }

void loop()
{
  int      ret;
  int      subid;
  int8_t   msgid;
  MyPacket *packet;

  wait_command_input();
  /* Receive message from SubCore */
  for (subid = 1; subid <= 5; subid++) {
    ret = MP.Recv(&msgid, &packet, subid);
    if (ret > 0) {
      printf("%s\n", packet->message);
      /* status -> ready */
      packet->status = 0;
    }
  }
}
