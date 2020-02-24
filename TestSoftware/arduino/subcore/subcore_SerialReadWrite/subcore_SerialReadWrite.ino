#include <MP.h>
#if (SUBCORE != 1)
#error "Core selection is wrong"
#endif

void setup()
{
  // Start the built-in serial port, probably to Serial Monitor
  int bundrate = 115200;
  Serial.begin(bundrate);
  if(!Serial){
    return ;
  }

  int ret;
  ret = MP.begin();
  MPLog("Start subcore1=%d\n",ret);

  MP.EnableConsole();


  delay(1000);
  Serial.write('n');
  Serial.write('A');
  Serial.write('0');

  Serial.write("He5$\n");  
  Serial.write("He5$\n",3); 
  Serial.println();
  Serial.print(78,OCT); 
  Serial.println();
  Serial.print(78,BIN); 
  Serial.println();
  Serial.print(78,DEC); 
  Serial.println();
  Serial.print(78,HEX); 
  Serial.println();
  Serial.print(1.23456,5);
  Serial.println();

}

void loop()
{
  // put your main code here, to run repeatedly:
  bool firstprint = true;
  while(Serial.available()>0){
    char buff = Serial.read();
    if(firstprint)
      Serial.write("Serial read = ");
    firstprint = false;
    Serial.write(buff);
    delay(20);
  }
}
