void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  printf("A0 voltage = %d\n",analogRead(A0));
  printf("A1 voltage = %d\n",analogRead(A1));
  printf("A2 voltage = %d\n",analogRead(A2));
  printf("A3 voltage = %d\n",analogRead(A3));
  printf("A4 voltage = %d\n",analogRead(A4));
  printf("A5 voltage = %d\n\n",analogRead(A5));
  sleep(1);
}
