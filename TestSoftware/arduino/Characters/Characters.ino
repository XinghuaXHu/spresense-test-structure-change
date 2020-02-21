/*
 * the Characters task aims at knowing what the char value is.
 *
 * basic testing methods are checking the return value
 * when using the functions with the parameter,that is
 * or is not,what you want.
 *
 * the functions list in detail:
 * test-task
 * isAlphaNumeric()           'a'~'z','A'~'Z'  '0'~'9'
 * isAlpha()                  'a'~'z','A'~'Z'
 * isAscii()                  0~255
 * isWhitespace()             ' ' '\f' '\n' '\r' '\t' '\v' and so on
 * isControl()                0~31 127
 * isDigit()                  '0'~'9'
 * isGraph()                  Printable with some content
 * isLowerCase()              'a'~'z'
 * isPrintable()              32~126
 * isPunct()                  [][!"#$%&'()*+,./:;<=>?@\^_`{|}~-]
 * isSpace()                  ' '
 * isUpperCase()              'A'~'Z'
 * isHexadecimalDigit()       'A'-'F', '0'-'9' 'a'-'f'
 *
 */

void setup() {
  // put your setup code here, to run once:
    /* TESTING isAlphaNumeric(x) */
    /* Parametre is */
    assert(isAlpha('a'));
    /* Parametre is not */
    assert(!(isAlpha(0)));

    /* TESTING isAlpha(x) */
    /* Parametre is */
    assert(isAlphaNumeric('0'));
    /* Parametre is not */
    assert(!(isAlphaNumeric(11)));

    /* TESTING isAscii(x) */
    /* Parametre is */
    assert(isAscii('a'));
    /* Parametre is not */
    assert(!(isAscii(1000)));

    /* TESTING isWhitespace(x) */
    /* Parametre is */
    assert(isWhitespace(' '));
    /* Parametre is not */
    assert(!(isWhitespace('a')));

    /* TESTING isControl(x) */
    /* Parametre is */
    assert(isControl(1));
    /* Parametre is not */
    assert(!(isControl('a')));

    /* TESTING isDigit(x) */
    /* Parametre is */
    assert(isDigit('1'));
    /* Parametre is not */
    assert(!(isDigit('a')));

    /* TESTING isGraph(x) */
    /* Parametre is */
    assert(isGraph('a'));
    /* Parametre is not */
    assert(!(isGraph(1)));

    /* TESTING isLowerCase(x) */
    /* Parametre is */
    assert(isLowerCase('a'));
    /* Parametre is not */
    assert(!(isLowerCase('A')));

    /* TESTING isPrintable(x) */
    /* Parametre is */
    assert(isPrintable(32));
    /* Parametre is not */
    assert(!(isPrintable(1)));

    /* TESTING isPunct(x) */
    /* Parametre is */
    assert(isPunct('['));
    /* Parametre is not */
    assert(!(isPunct('a')));

    /* TESTING isSpace(x) */
    /* Parametre is */
    assert(isSpace(' '));
    /* Parametre is not */
    assert(!(isSpace('a')));

    /* TESTING isUpperCase(x) */
    /* Parametre is */
    assert(isUpperCase('A'));
    /* Parametre is not */
    assert(!(isUpperCase('a')));

    /* TESTING isHexaDecimalDigit(x) */
    /* Parametre is */
    assert(isHexadecimalDigit('A'));
    /* Parametre is not */
    assert(!(isHexadecimalDigit(12)));
    
    printf("\nthe task Characters test ok!\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
