#include <SDHCI.h>
SDClass SD;

File testFile;
File T1st1;
File T2st2;
File myFile;
File myFile2;
File Dummy2;
File Sunny1;
File Dummy4;
File Sunny2;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial){
  }
  Serial.print("Initializing SD card...");
  if(!SD.begin()){
    Serial.println("SD begin failed!!");
    return;
  }
  Serial.println("initialization done.");

  if(SD.exists("test_done")){
      Serial.println("Already done!!");
      return;
  }

  // from sdcard_createnewfile_15643.ino -->
  testFile = SD.open("test",FILE_WRITE);
  if(testFile){
    printf("open file success\n");
    testFile.write("He2\n4h Q");
    testFile.close();
    printf("write to test done\n");
  }
  else{
    printf("open file failure\n");
  }
  // from sdcard_createnewfile_15643.ino <--

  // from sdcard_createnewfile_15645.ino -->
    T1st1 = SD.open("T1st1",FILE_WRITE);
  if(T1st1){
    printf("open file success\n");
    T1st1.write("6G n\n");
    
    printf("write to T1st1 done\n");
  }
  else{
    printf("open file failure\n");
  }
    
  T2st2 = SD.open("T2st2",FILE_WRITE);
  T1st1.close();
  if(T2st2){
    printf("open file success\n");
    T2st2.write("3 ffG");
    T2st2.close();
    printf("write to T2st2 done\n");
  }
  else{
    printf("open file failure\n");
  }
  // from sdcard_createnewfile_15645.ino <--  

  // from sdcard_write_15648.ino -->
  File Dummy1 = SD.open("Dummy1",FILE_WRITE);
  if(Dummy1){
    printf("write file\r\n");
    Dummy1.write('d');
    Dummy1.write('\n');
    Dummy1.write('0');
    Dummy1.write(' ');
    Dummy1.write('$');
    Dummy1.write('0');
    Dummy1.write('R');

    Dummy1.write("4dG&\na P");
    Dummy1.write("");

    char buf[20] = "He\n4 #";
    Dummy1.write(buf,7);
    strcpy(buf,"fn54dnh #");
    Dummy1.write(buf,4);
    strcpy(buf,"");
    Dummy1.write(buf,0);

    Dummy1.close();
    printf("write to Dummy1 done\n");
  }
  else{
    printf("open file error\r\n");
  }
  // from sdcard_write_15648.ino <--

  // from sdcard_write_15650.ino -->
  Dummy2 = SD.open("Dummy2", FILE_WRITE);
  // if the file opened okay, write to it:
  if (Dummy2) {
    Dummy2.seek(2);
    Dummy2.write("4g()\n");
    Dummy2.close();
    Serial.println("write to Dummy2 done");
  } else {
    // if the file didn't open, print an error:
    Serial.println("open file error\r\n");
  }
  Sunny1 = SD.open("Sunny1", FILE_WRITE);
  // if the file opened okay, write to it:
  if (Sunny1) {
    Sunny1.write("#$nm");
    Sunny1.close();
    Serial.println("write to Sunny1 done");
  } else {
    // if the file didn't open, print an error:
    Serial.println("open file error\r\n");
  }
  // from sdcard_write_15650.ino <--

  // from sdcard_write_15651.ino -->
  myFile2 = SD.open("Dummy3", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile2) {
    myFile2.seek(0);
    myFile2.write(" A");
    myFile2.seek(5);
    myFile2.write("#g");
    Serial.print("file size:");
    Serial.println(myFile2.size());
    myFile2.seek(myFile2.size());
    myFile2.write("1\n");
    myFile2.close();
    Serial.println("write to Dummy3 done");
  } else {
    // if the file didn't open, print an error:
    Serial.println("open file error\r\n");
  }
  // from sdcard_write_15651.ino <--

  // from sdcard_write_15652.ino -->
   Dummy4 = SD.open("Dummy4", FILE_WRITE);
  if (Dummy4) {
    Dummy4.seek(5);
    Dummy4.write("ak47kill");

    Serial.println("write to Dummy4 done");
  } else {
    // if the file didn't open, print an error:
    Serial.println("open file error");
  }
   Sunny2 = SD.open("Sunny2", FILE_WRITE);
  if (Sunny2) {
    Sunny2.write("trash");
    
    Serial.println("write to Sunny2 done");
  } else {
    // if the file didn't open, print an error:
    Serial.println("open file error");
  }
  Dummy4.close();
  Sunny2.close();
  // from sdcard_write_15652.ino <--

  File testDone = SD.open("test_done", FILE_WRITE);
  testDone.write("test_done");
  testDone.close();
  Serial.println("sd_create_write test done");
}

void loop() {
  // put your main code here, to run repeatedly:
}
