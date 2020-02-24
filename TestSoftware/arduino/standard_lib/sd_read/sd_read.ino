#include <SDHCI.h>
SDClass SD;

File Dummy;
File Sunny;
char buffer1[32];
char buffer2[32];
File test;
char buf[32];

// sdcard_read_15653.ino
void read_arbitrary() {
  File myFile;
  //open the file named "Dummy" using read mode
  myFile = SD.open("Dummy");
  if(myFile){
    Serial.println("open file success");
    char buffer[10];
      
    //use function of read() to read the one byte of  the file. Then use assert to verify that the one byte read is equal to 't'.
    myFile.read(buffer,1);
    printf("read buffer 1:%c\n",buffer[0]);
    
    myFile.seek(3);
    //use function of read(buffer, 3) to read the 3 characters of the file into buffer
    myFile.read(buffer,3);  
    printf("read buffer 3:%s\n",buffer);
    
    //seek the reading postion to the final postion vailable for read which is the size of the file minus 1.
    int file_size = myFile.size();
    printf("file size:%d\n",file_size);
    myFile.seek(file_size-1);
   
   //use function of read() to read the one byte of  the file. Then use assert to verify that the one byte read is equal to 'n'.
    myFile.read(buffer,1);
    printf("read buffer postion %d:%c\n",file_size-1,buffer[0]);
    assert(buffer[0] == 'n');
    printf("read passed\n");
    myFile.close();
  }
  else{
    Serial.println("open file error\n");
  }
}

// sdcard_read_15655.ino
void read_simultaneously() {
  Dummy = SD.open("Dummy1");
  Sunny = SD.open("Sunny1");
  
  if(Dummy){
    printf("open Dummy file success\n");
    Dummy.read(buffer1,3);
    printf("Dummy read buffer1:%s\n",buffer1);
    printf("done\n");
  }else{
    Serial.println("open file error");
  }

  if(Sunny){
    printf("open Sunny file success\n");
    Sunny.seek(1);
    printf("set seek 1 passed\n");
    Sunny.read(buffer2,7);
    for(int i=0;i<7;i++)
      printf("Sunny read buffer2:%d\n",buffer2[i]);
  }else{
    Serial.println("open file error");
  }
  Dummy.close();
  Sunny.close();
}

// sdcard_read_15656.ino
void read_newly() {
  // Remove test file before testing
  SD.remove("test.txt");

  test = SD.open("test.txt",FILE_WRITE);
  
  if(test){
    printf("open test file success\n");
    test.write("\n1dH #$");
    test.flush();
    test.seek(0);
    test.read(buf,test.size());
    printf("read buf:%s\n",buf);
    test.close();
    printf("done\n");
  }else{
    Serial.println("open file error\n");
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial){ 
  }
  Serial.println("initializing SD card ...");
  Serial.println("initialization done...");
  Serial.println("sd_read test start");
  Serial.println("read_arbitrary test start");
  read_arbitrary();
  Serial.println("read_arbitrary test done");
  Serial.println("read_simultaneously test start");
  read_simultaneously();
  Serial.println("read_simultaneously test done");
  Serial.println("read_newly test start");
  read_newly();
  Serial.println("read_newly test done");
  // put your setup code here, to run once:
  Serial.println("sd_read test done");
}

void loop() {
  // put your main code here, to run repeatedly:
}
