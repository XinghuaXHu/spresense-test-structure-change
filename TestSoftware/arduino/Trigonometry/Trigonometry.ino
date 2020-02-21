/*
 * the Trigonometry task aims at doing some simple calculations
 * about .
 *
 * basic testing methods are checking the return value 
 * when using the functions with the parameters including 
 * the max,the min,the mid,and the error.
 *
 * the functions list in detail:
 * test-task        Syntax              correct-return
 *
 *  sin()           sin(rad)
 *  cos()           cos(rad)
 *  tan()           tan(rad)
 *
 *please confirm with calculator,if you want to test other value using sin()
 */
bool functions_test_sin(float cal, float reval)
{
    if(abs(reval - sin(cal)) > 0.0001)
    {
        return 0;
    }
    return 1;
}

bool functions_test_cos(float cal, float reval)
{
    if(abs(reval - cos(cal)) > 0.0001)
    {
        return 0;
    }
    return 1;
}

bool functions_test_tan(float cal, float reval)
{
    if(abs(reval - tan(cal)) > 0.0001)
    {
        return 0;
    }
    return 1;
}

void setup() {
  // put your setup code here, to run once:

   /* TESTING sin(rad) */
   /* 0   degree */
   assert(functions_test_sin(0, 0));
   /* 45  degree */
   assert(functions_test_sin(0.785398163, 0.707106781));
   /* 90  degree */
   assert(functions_test_sin(1.570796327, 1));
   /* 120 degree */
   assert(functions_test_sin(2.094395102, 0.866025404));
   /* 180 degree */
   assert(functions_test_sin(3.141592653, 0));
   /* 270 degree */
   assert(functions_test_sin(4.712388979, -1));

   /* TESTING cos(rad) */
   /* 0   degree */
   assert(functions_test_cos(0, 1));
   /* 45  degree */
   assert(functions_test_cos(0.785398163, 0.707106781));
   /* 90  degree */
   assert(functions_test_cos(1.570796327, 0));
   /* 120 degree */
   assert(functions_test_cos(2.094395102, -0.5));
   /* 180 degree */
   assert(functions_test_cos(3.141592653, -1));
   /* 270 degree */
   assert(functions_test_cos(4.712388979, 0));

   /* TESTING tan(rad) */
   /* 0   degree */
   assert(functions_test_tan(0, 0));
   /* 45  degree */
   assert(functions_test_tan(0.785398163, 1));
   /* 120 degree */
   assert(functions_test_tan(2.094395102, -1.7320508));
   /* 180 degree */
   assert(functions_test_tan(3.141592653, 0));

   printf("\nthe task Trigonometry test ok!\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
