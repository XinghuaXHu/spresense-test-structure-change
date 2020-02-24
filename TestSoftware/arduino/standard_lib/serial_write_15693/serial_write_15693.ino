void setup() {
  // put your setup code here, to run once:
  //begin the serial with rate of 115200.
  Serial.begin(115200);
  //Sparduino uses write(val) function to send one byte 'n','A','0','\n','$' into port respectively.
  Serial.write('n');
  Serial.write('A');
  Serial.write('0');
  Serial.write('\n');
  Serial.write('$');
  //use the wirte("He5$\n") function to send a string
  Serial.write("He5$\n");
  //use the write(buf, len) fucntion  to send char array "4aG\n N". the len is 4
  char buf[10] = "4aG\n N";
  Serial.write((const uint8_t*)buf,4);
}

void loop() {
  // put your main code here, to run repeatedly:

}
