#include <stdio.h>
#include <Watchdog.h>

#include <RTC.h>
#include <MP.h>
#if (SUBCORE != 1)
#error "Core selection is wrong"
#endif

#define BAUDRATE                (115200)

#define ON_SUBCORE

void setup() {
  int ret;
  
  Serial.begin(BAUDRATE);
  while (!Serial)
    {
      ; /* wait for serial port to connect. Needed for native USB port only */
    }

#ifdef ON_SUBCORE
  ret = MP.begin();
  MPLog("Start subcore1=%d\n",ret);
#endif
    
  MPLog("Start Watchdog test\n");

  //Init watchdog
  Watchdog.begin(); 
  MPLog("Watchdog: begin()\n");

  //Start watchdog
  Watchdog.start(30000);
}

void loop()
{  
  MPLog("%d ms left for watchdog bite\n", Watchdog.timeleft() );
  delay(1000);
}
   
