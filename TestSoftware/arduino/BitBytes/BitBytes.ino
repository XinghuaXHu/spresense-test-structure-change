/*
 * the Characters task aims at knowing what the char value is.
 *
 * basic testing methods are checking the return value
 * when using the functions with the parameter,that is
 * or is not,what you want.
 *
 * the functions list in detail:
 * test-task        Syntax                return                         function
 *
 * lowByte()      lowByte(x)             byte                            Extracts the low-order (rightmost)
 *                                                                       byte of a variable (e.g. a word)
 * highByte()     highByte(x)            The value of the bit            Extracts the high-order (leftmost)
 *                                                                       byte of a word (or the second
 *                                                                       lowest byte of a larger data type).
 * bitRead()      bitRead(x, n)          the value of the bit (0 or 1)   Reads a bit of a number.
 * bitWrite()     bitWrite(x, n, b)      Nothing                         Writes a bit of a numeric variable.
 * bitSet()       bitSet(x, n)           Nothing                         Sets (writes a 1 to) a bit of a numeric
 *                                                                       variable.
 * bitClear()     bitClear(x, n)         Nothing                         Clears (writes a 0 to) a bit of a numeric
 *                                                                       variable.
 * bit()          bit(n)                 The value of the bit            Computes the value of the specified bit
 *                                                                       (bit 0 is 1, bit 1 is 2, bit 2 is 4, etc.).
 */

bool functions_test_lowByte(unsigned int val)
{
    unsigned int tmp = 0xff;

    if((tmp & val) != lowByte(val))
    {
        return 0;
    }
    return 1;
}

bool functions_test_highByte(unsigned int val)
{
    unsigned int tmp = 0xff;

    if((tmp & (val>>8)) != highByte(val))
    {
        return 0;
    }
    return 1;
}

bool functions_test_bitRead(unsigned int val,byte n)
{
    unsigned int tmp = 0x1;

    if((tmp & (val>>n)) != bitRead(val,n))
    {
        return 0;
    }
    return 1;
}

bool functions_test_bitWrite(unsigned int val,byte n,unsigned int b)
{
    unsigned int tmp = 0x1;
    unsigned int v = val;
    bitWrite(v,n,b);
    bool con = ((tmp & (v >> n)) == (b));

    if(!con)
    {
        return 0;
    }
    return 1;
}

bool functions_test_bitSet(unsigned int val,byte n)
{
    unsigned int tmp = 0x1;
    unsigned int v = val;
    bitSet(v,n);
    bool con = ((tmp & (v >> n)) == 0x1 );

    if(!con)
    {
        return 0;
    }
    return 1;
}

bool functions_test_bitClear(unsigned int val,byte n)
{
    unsigned int tmp = 0x1;
    unsigned int v = val;
    bitClear(v,n);
    bool con = ((tmp & (v >> n)) == 0x0 );

    if(!con)
    {
        return 0;
    }
    return 1;
}

bool functions_test_bit(byte n)
{
    unsigned int tmp = pow(2,n);

    if(tmp != bit(n))
    {
        return 0;
    }
    return 1;
}

void setup() {
  // put your setup code here, to run once:
    /* TESTING lowByte(x) */
    assert(functions_test_lowByte(0xABCD));

    /* TESTING highByte(x) */
    assert(functions_test_highByte(0xABCDEF12));

    /* TESTING bitRead(x,n) */
    assert(functions_test_bitRead(0x55555555,5));

    /* TESTING bitWrite(x, n, b) */
    assert(functions_test_bitWrite(0xFFFFFFFF,5,0));

    /* TESTING bitSet(x, n) */
    assert(functions_test_bitSet(0x0,2));

    /* TESTING bitClear(x, n) */
    assert(functions_test_bitClear(0xFFFF,2));

    /* TESTING bit(n) */
    assert(functions_test_bit(2));

    printf("\nthe task BitandBytes test ok!\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
