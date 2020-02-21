#include <MP.h>
#ifdef SUBCORE
#error "Core selection is wrong"
#endif

void setup() {
  int ret;
  
  // put your setup code here, to run once:
  Serial.begin(115200);

  while(!Serial){}

  MPLog("Start Main core!\n");
   
  ret = MP.begin(1);
  MPLog("Begin subcore1=%d\n",ret);
  
}

void loop() {


}
