void testPingSet(int pin, int value)
{
  uint8_t pins[] =
  {
    PIN_D00, PIN_D01, PIN_D02, PIN_D03,
    PIN_D04, PIN_D05, PIN_D06, PIN_D07,
    PIN_D08, PIN_D09, PIN_D10, PIN_D11, PIN_D12
  };
  pinMode(pins[pin], OUTPUT);
  digitalWrite(pins[pin], value);
}

void setup()
{

  // put your setup code here, to run once:
  int bundrate = 115200;
  Serial.begin(bundrate);
  if(!Serial)
  {
    return ;
  }

  Serial2.begin(bundrate);
  if(!Serial)
  {
    Serial2 ;
  }
  testPingSet(2, HIGH);
  testPingSet(3, HIGH);
  testPingSet(4, HIGH);
  testPingSet(5, HIGH);
  testPingSet(6, HIGH);
  testPingSet(7, HIGH);
  testPingSet(10, HIGH);
  testPingSet(11, HIGH);
  testPingSet(12, HIGH);
}
char rbuf[128] = {0,};
int rbufIndex = 0;
void loop()
{
  // put your main code here, to run repeatedly:
  while (Serial.available() > 0)
  {
    char inByte = Serial.read();
    rbuf[rbufIndex] = inByte;
    rbufIndex++;
    rbuf[rbufIndex] = 0;
    if(inByte == 13 || inByte == 10)
    {
      if((rbuf[1] == 'l') && (rbuf[0] >= '0') && (rbuf[0] <= '9'))
        testPingSet(rbuf[0] - '0', LOW);
      else if((rbuf[1] == 'h') && (rbuf[0] >= '0') && (rbuf[0] <= '9'))
        testPingSet(rbuf[0] - '0', HIGH);
      else if((rbuf[1] == 'l') && (rbuf[0] >= 'a') && (rbuf[0] <= 'z'))
        testPingSet(rbuf[0] - 'a' + 10, LOW);
      else if((rbuf[1] == 'h') && (rbuf[0] >= 'a') && (rbuf[0] <= 'c'))
        testPingSet(rbuf[0] - 'a' + 10, HIGH);
      else
      {
        int loop = 0;
        while(loop < rbufIndex)
        {
          Serial2.write(rbuf[loop]);
          loop++;
        }
      }
      rbufIndex = 0;
    }
  }
  while (Serial2.available() > 0)
  {
    char inByte = Serial2.read();
    Serial.write(inByte);
  }
}
