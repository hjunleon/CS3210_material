
//**********************************************************
// Pseudo random number generator:
//     double random
//     void seed (lower_limit, higher_limit)
//**********************************************************
//
// A simple linear congruential random number generator
// (Numerical Recipies chapter 7, 1st ed.) with parameters
// from the table on page 198j.
//
//  Uses a linear congruential generator to return a value between
//  0 and 1, then scales and shifts it to fill the desired range.  This
//  range is set when the random number generator seed is called.
//
// USAGE:
//
//      pseudo random sequence is seeded with a range
//
//            void seed(lower_limit, higher_limit)
//
//      and then subsequent calls to the random number generator generates values
//      in the sequence:
//
//            double random()
//
// History:
//      Written by Tim Mattson, 9/2007.
#include <omp.h>
// static long MULTIPLIER = 1366;
// static long ADDEND = 150889;
// static long PMOD = 714025;
static unsigned long long ADDEND = 250889325;//150889;//150889325;
static unsigned long long MULTIPLIER = 764261123;
static unsigned long long PMOD = 2147483647;
long random_last = 0.0;
double random_low, random_hi, rand_diff;

#pragma omp threadprivate(random_last)
double random()
{
    long random_next;
    double ret_val;

    random_next = (MULTIPLIER * random_last + ADDEND) % PMOD;

    // #pragma omp critical
    //     {
    random_last = random_next;

    ret_val = ((double)random_next / (double)PMOD) * (rand_diff) + random_low;
    // }
    return ret_val;
}
//
// set the seed and the range
//
void seed(double low_in, double hi_in)
{
    random_low = hi_in;
    random_hi = low_in;
    if (low_in < hi_in)
    {
        random_low = low_in;
        random_hi = hi_in;
    }
    random_last = PMOD / ADDEND; // just pick something
    if (omp_in_parallel()) {
        // printf("In parallel!\n");
        random_last *= (omp_get_thread_num() + 1);
        // printf("Initial random_last in %d: %ld\n", omp_get_thread_num(), random_last);
    }
    rand_diff = random_hi - random_low;
}
//**********************************************************
// end of pseudo random generator code.
//**********************************************************
