/*
 * the Math task aims at doing some simple calculations 
 * about math.
 * 
 * basic testing methods are checking the return value 
 * when using the functions with the parameters including 
 * the max,the min,the mid,and the error.
 * 
 * the functions list in detail:
 * test-task        Syntax              correct-return
 *                                      signed/unsigned
 *  min()           min(x, y)       ，   the smaller one
 *  max()           max(x, y)           the bigger one
 *  abs()           abs(x)              |x|
 *  constrain()     constrain(x, a, b)  between a and b
 *  map()           map(x, fL, fH, tL, tH)
 *  pow()           pow(base, exponent)
 *  sqrt()          sqrt(x)
 */

 
 
/*
* functions_test_min(float maxv,float minv).
* description:
*   These functions test whether the results calculated by min() is right.
*
* Parameters:
*   maxv: the bigger one .
*   minv: the smaller one.
*
* Return value:
*   return ture if the result calculated by min() is correct,or return false.
*/
bool functions_test_min(float maxv, float minv)
{
    float tmp;
    tmp = ((maxv >= minv) ? minv : maxv);

    if(tmp != min(maxv,minv))
    {
        return 0;
    }
    return 1;
}

/*
* functions_test_max(float maxv,float minv).
* description:
*   These functions test whether the results calculated by max() is right.
*
* Parameters:
*   maxv: the bigger one.
*   minv: the smaller one.
*
* Return value:
*   return ture if the result calculated by max() is correct,or return false.
*/
bool functions_test_max(float maxv,float minv)
{
    float tmp;
    tmp = ((maxv >= minv) ? maxv : minv);

    if(tmp != max(maxv,minv))
    {
        return 0;
    }
    return 1;
}

/*
* functions_test_abs(float cal,float reval).
* description:
*   These functions test whether the results calculated by abs() is right.
*
* Parameters:
*   cal: the number.
*   reval: the smallest.
*
* Return value:
*   return ture if the result calculated by abs() is correct,or return false.
*/
bool functions_test_abs(float cal,float reval)
{
    if(reval != abs(cal))
    {
        return 0;
    }
    return 1;
}

/*
* functions_test_constrain(float val,float bottom,float top).
* description:
*   These functions test whether the results calculated by constrain() is right.
*
* Parameters:
*   cal: the number.
*   bottom: the smallest.
*   top:the biggest.
*
* Return value:
*   return ture if the result calculated by constrain() is correct,or return false.
*/
bool functions_test_constrain(float cal,float bottom,float top)
{
    float tmp;
    tmp = constrain(cal,bottom,top);
    if(tmp > top || tmp < bottom)
    {
        return 0;
    }
    return 1;
}

/*
* functions_test_map(long value, long fromLow,long fromHigh,long toLow,long toHigh).
* description:
*   These functions test whether the results converting the range value is right.
*
* Parameters:
*   value: Value to convert.
*   fromLow: Lower limit of the value before conversion.
*   fromHigh: Higher limit of the value before conversion.
*   toLow: Lower limit of the value after conversion.
*   toHigh: Higher limit of the value after conversion.
*
* Return value:
* return ture if the result calculated between toLow and toHigh,or return false.
*
* please confirm with calculator,if you want to test other value using map().
*/
bool functions_test_map(long value,long fromLow,long fromHigh,long toLow,long toHigh)
{
    long tmp;
    tmp = map(value,fromLow,fromHigh,toLow,toHigh);
    if(toLow < toHigh){
      if(tmp<toLow || tmp>toHigh){
          return 0;
      }      
    }else if(toLow > toHigh){
      if(tmp>toLow || tmp<toHigh){
          return 0;
      }
    }
    return 1;
}

/*
* functions_test_pow(float cal,float reval).
* description:
*   These functions test whether the results calculated by pow is right.
*
* Parameters:
*   base: the number.
*   exponent: the power to which the base is raised.
*   reval:the real value you get using calculator.
*
* Return value:
* return ture if the result calculated by sqrt is same to yours,or return false.
*
* please confirm with calculator,if you want to test other value using pow().
*/
bool functions_test_pow(float base,float exponent,float reval)
{
    if(abs(reval - pow(base,exponent)) > 0.0001)
    {
        return 0;
    }
    return 1;
}

/*
* functions_test_sqrt(float cal,float reval).
* description:
*   These functions test whether the results calculated by sqrt is right.
*
* Parameters:
*   cal:which you want to calculate with sqrt.
*   reval:the real value you get using calculator.
*
* Return value:
*   return ture if the result calculated by sqrt is same to yours,or return false.
*
* please confirm with calculator,if you want to test other value using sqrt().
*/
bool functions_test_sqrt(float cal,float reval)
{
    if(abs(reval - sqrt(cal)) > 0.0001)
    {
        return 0;
    }
    return 1;
}


void setup() {
  // put your setup code here, to run once:

   /* TESTING min(x, y) */;
   /* Different parametre */
   assert(functions_test_min(20.1,15.6));
   /* Equal parametre */
   assert(functions_test_min(1,1));

   /* TESTING max(x, y) */;
   /* Different parametre */
   assert(functions_test_max(88,66));
   /* Equal parametre */
   assert(functions_test_max(1,1));

   /* TESTING abs(x) */
   /* Zero parametre */
   assert(functions_test_abs(0,0));
   /* Negative parametre */
   assert(functions_test_abs(-10.1,10.1));
   /* Positive parametre */
   assert(functions_test_abs(10.1,10.1));

   /* TESTING constrain(x, a, b) */
   /* Less than bottom value */
   assert(functions_test_constrain(10.5, 22.1, 33.8));
   /* Greater than top value */
   assert(functions_test_constrain(50, 22.1, 33.8));
   /* Middle value */
   assert(functions_test_constrain(25, 22.1, 33.8));

   /* TESTING map(x, fL, fH, tL, tH) */
   /* Less than bottom value */
   assert(functions_test_map(1, 1, 50, 50, 1));
   /* Negative values are also included in the range */
   assert(functions_test_map(50, 1, 50, 50, -100));
   /* Middle value */
   assert(functions_test_map(25, 1, 50, 50, 1));
   
   /* TESTING pow(base, exponent) */
   /* Zero test */
   assert(functions_test_pow(5, 0, 1));
   /* Middle parametre test */
   assert(functions_test_pow(5, 2.5, 55.90170));
   /* Negative parametre test */
   assert(functions_test_pow(5, -2.5, 0.01789));
   /* Max parametre test */
   //assert(functions_test_pow(2, 32, 429496729));

   /* TESTING sqrt(x) */
   /* Zero test */
   assert(functions_test_sqrt(0, 0));
   /* Middle parametre test */
   assert(functions_test_sqrt(5, 2.236067977));

   printf("\nthe task Math test ok!\n");

}

void loop() {
  // put your main code here, to run repeatedly:

}
