/*
 * the Random Numbers task aims at generating pseudo-random numbers.
 *
 * basic testing methods are checking whether the number you get this
 * time is same as the number you got last.
 *
 * the functions list in detail:
 * test-task          Syntax                        return
 * random()      random(max)/random(min, max)     A random number between min and max-1 (long)
 * randomSeed()  randomSeed(long/int)               nothing
 */

bool functions_test_random_randomSeed(long seed,int times)
{
    long randNumber1[200],randNumber2[200];
    int maxv;
    int i;

    if(times > 200)
    {
        times = 200;
    }
    maxv = times;
    randomSeed(seed);
    for(i = 0;i < times;i++)
    {
        randNumber1[i] = random(maxv);
    }
    randomSeed(seed);
    for(i = 0;i < times;i++)
    {
        randNumber2[i] = random(maxv);
    }
    for(i = 0;i < times;i++)
    {
        if(randNumber1[i] != randNumber2[i])
        {
            return 0;
        }
    }
    return 1;
}



void setup() {
  // put your setup code here, to run once:
    assert(functions_test_random_randomSeed(500,100));
    printf("\nRandom Numbers the task Conversion test ok!\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
