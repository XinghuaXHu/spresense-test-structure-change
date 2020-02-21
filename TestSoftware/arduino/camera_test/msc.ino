#include <SDHCI.h> 
SDClass SD;
void init_msc(void)
{
  /* Initialize eMMC */
  while (!SD.begin()){
    Serial.println("Wait until a SD card is mounted ");
    delay(500);
  }

  /* Start USB MSC */
  if (SD.beginUsbMsc()) {
    Serial.println("USB MSC Start Failure!");
  } else {
    Serial.println("*** USB MSC Prepared! ***");
    Serial.println("Connect Extension Board USB to PC.");
  }  
}

void fin_msc(void)
{
  /* Stop USB MSC */
  if (SD.endUsbMsc()) {
    Serial.println("USB MSC End Failure!");
  } else {
    Serial.println("*** USB MSC Ended! ***");
    Serial.println("Finish USB Mass Storage Operation");
  }  
}
