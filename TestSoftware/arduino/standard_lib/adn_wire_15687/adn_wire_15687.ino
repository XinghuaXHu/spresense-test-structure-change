#include <Wire.h>

uint8_t address = 0x08;

void wire_send_byte(char ch){
  //Sparduino uses function beginTransmission(8) to begin a transmission to the I2C slave device with address of 8.
  //Sparduino  uses function write('a') to send charactor 'a'
  //Then use endTransmission() to end this transmisstion.
  Wire.beginTransmission(address);
  Wire.write(ch);
  Wire.endTransmission();
}

void wire_send_str(char* str){
  Wire.beginTransmission(address);
  Wire.write(str);
  Wire.endTransmission();
}

void wire_send_bytes(byte* by,int len){
  Wire.beginTransmission(address);
  Wire.write(by,len);
  Wire.endTransmission();
}
void setup() {
  Wire.begin();
  Serial.begin(115200);
  if(!Serial){
    return ;
  }
  // put your setup code here, to run once:
  //Sparduino :use function begin() to start the I2C as master device.
  //Arduino Uno :use begin(8) to start the I2C as slave device with address 8
  Wire.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  wire_send_byte('a');
  wire_send_byte('B');
  wire_send_byte(' ');
  wire_send_byte('\0');
  wire_send_byte('\n');
  wire_send_byte('%');
  wire_send_byte('0');

  //wire_send_str("Ax# n\n0");
  Wire.beginTransmission(address);
  Wire.write("Ax# n\n0");
  Wire.endTransmission();

  byte data[16] = "Ax# n\n0";
  wire_send_bytes(data,5);
}
