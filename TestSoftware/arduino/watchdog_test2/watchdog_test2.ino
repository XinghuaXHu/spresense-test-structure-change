/*
* Test sketch for
* spresense-16384
*/

#include <Watchdog.h>

#define BAUDRATE                (115200)

void setup() {
  Serial.begin(BAUDRATE);
  while (!Serial)
    {
      ; /* wait for serial port to connect. Needed for native USB port only */
    }
  Serial.println("Start Watchdog test");
  Watchdog.begin();
  Watchdog.start(30000);
}

void loop() {
  int i;
  for(i = 0; i<10; i++)
  {
    usleep(1000 * 1000 * 3);
    Serial.println( Watchdog.timeleft() ) ;
    Serial.println("ms left for watchdog bite(before kick)");
    Watchdog.kick();
    Serial.println( Watchdog.timeleft() ) ;
    Serial.println("ms left for watchdog bite(after kick)");
    if( i == 9 ) Watchdog.stop();
  }
  Serial.println( "end kick-loop and reset watchdog" ) ;
  Watchdog.start(40000);
  usleep(1000 * 1000 * 45);
}
