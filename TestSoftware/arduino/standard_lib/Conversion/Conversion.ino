/*
 * the Conversion task aims at turning a value of
 * any type to the special type.
 *
 * basic testing methods are checking the return value
 * when using the functions with the parameters including
 * the max,the min,the mid,the error and the overflow.
 *
 * the functions list in detail:
 * test-task        Syntax           correct-parameter              correct-return
 *   char()         char(x)             0~255                           char
 *   byte()         byte(x)             0~255                           byte
 *   word()         word(x)/word(h,l)   0~65535                         word
 *   int()          int(x)              -2,147,483,648~2,147,483,647    int
 *   long()         long(x)             -2,147,483,648~2,147,483,647    long
 *   float()        float(x)            -+3.4028235 E+38                float
 */
#define CHAR_MAX_VAL  CHAR_MAX
#define CHAR_MIN_VAL  CHAR_MIN
#define BYTE_MAX_VAL  (255)
#define BYTE_MIN_VAL  (0)
#define WORD_MAX_VAL  (65535)
#define WORD_MIN_VAL  (0)
#define INT_MAX_VAL   INT_MAX
#define INT_MIN_VAL   INT_MIN
#define LONG_MAX_VAL  LONG_MAX
#define LONG_MIN_VAL  LONG_MIN
#define FLOAT_MAX_VAL ((1 - pow(2,-24)) * pow(2,128))
#define FLOAT_MIN_VAL (-((1 - pow(2,-24)) * pow(2,128)))

bool functions_test_char(int cal,int reval)
{
    int ret = 0;
    ret = char(cal);

    if(ret != reval)
    {
        return 0;
    }
    return 1;
}

bool functions_test_byte(int cal,int reval)
{
    int ret = 0;
    ret = byte(cal);

    if(ret != reval)
    {
        return 0;
    }
    return 1;
}

bool functions_test_word(int cal,int reval)
{
    int ret = 0;
    ret = word(cal);

    if(ret != reval)
    {
        return 0;
    }
    return 1;
}

bool functions_test_int(int cal, int reval)
{
    int ret = 0;
    ret = int(cal);

    if(ret != reval)
    {
        return 0;
    }
    return 1;
}

bool functions_test_long( long cal, long reval)
{
    int ret = 0;
    ret = long(cal);

    if(ret != reval)
    {
        return 0;
    }
    return 1;
}

bool functions_test_float(float cal,float reval)
{
    float ret,val;
    ret = float(cal);
    val = abs(ret-reval);

    if(val > 0.0001)
    {
        return 0;
    }
    return 1;
}

void setup() {

  // put your setup code here, to run once:

  /* TESTING char() */
  /* The smallest parametre (Zero) */
  assert(functions_test_char(CHAR_MIN_VAL, CHAR_MIN_VAL));
  /* The biggest parametre */
  assert(functions_test_char(CHAR_MAX_VAL, CHAR_MAX_VAL));
  /* The middle parametre */
  assert(functions_test_char(10, 10));
  /* The overflow parametre */
  assert(functions_test_char(300, (300%(CHAR_MAX_VAL+1))));
  /* The error parametre */
  assert(functions_test_char(-2, (-2+CHAR_MAX_VAL+1)));

  /* TESTING byte() */
  /* The smallest parametre (Zero) */
  assert(functions_test_byte(BYTE_MIN_VAL, BYTE_MIN_VAL));
  /* The biggest parametre */
  assert(functions_test_byte(BYTE_MAX_VAL, BYTE_MAX_VAL));
  /* The middle parametre */
  assert(functions_test_byte(10, 10));
  /* The overflow parametre */
  assert(functions_test_byte(300, (300%(BYTE_MAX_VAL+1))));
  /* The error parametre */
  assert(functions_test_byte(-1, (-1+CHAR_MAX_VAL+1)));

  /* TESTING word() */
  /* The smallest parametre (Zero) */
  assert(functions_test_word(WORD_MIN_VAL,WORD_MIN_VAL));
  /* The biggest parametre */
  assert(functions_test_word(WORD_MAX_VAL, WORD_MAX_VAL));
  /* The middle parametre */
  assert(functions_test_word(100, 100));
  /* The overflow parametre */
  assert(functions_test_word(70000, 70000%(WORD_MAX_VAL+1)));
  /* The error parametre */
  assert(functions_test_word(-1, (-1+WORD_MAX_VAL+1)));

  /* TESTING int() */
  /* The smallest parametre  */
  assert(functions_test_int(INT_MIN_VAL, INT_MIN_VAL));
  /* The biggest parametre */
  assert(functions_test_int(INT_MAX_VAL, INT_MAX_VAL));
  /* The middle parametre */
  assert(functions_test_int(10, 10));
  /* The error parametre */
  assert((int(-1.8) == (-1)));

  /* TESTING long() */
  /* The smallest parametre  */
  assert(functions_test_long(LONG_MIN_VAL, LONG_MIN_VAL));
  /* The biggest parametre */
  assert(functions_test_long(LONG_MAX_VAL, LONG_MAX_VAL));
  /* The middle parametre */
  assert(functions_test_long(10, 10));
  /* The error parametre */
  assert((long(-1.8) == (-1)));

  /* TESTING float() */
  /* The smallest parametre  */
  assert(functions_test_float(FLOAT_MAX_VAL, FLOAT_MAX_VAL));
  /* The biggest parametre */
  assert(functions_test_float(FLOAT_MIN_VAL, FLOAT_MIN_VAL));
  /* The middle parametre */
  assert(functions_test_float(1.123456789, 1.123456789));

  printf("\nthe task Conversion test ok!\n");

}

void loop() {


}
