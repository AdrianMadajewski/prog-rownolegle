#include "utility.h"

void printPrimes(array_t *primes, num_t size, num_t shift)
{
    int count = 0;
    for(int i = 0; i < size; i++)
    {
        if(primes[i] == 1)
        {
            if(count++ == 10)
            {
                printf("\n");
                count %= 10;
            }

            printf("%llu\t", i + shift - 1);
        }
    }
    printf("\n");
}

num_t countPrimes(array_t *primes, num_t size)
{
    num_t prime_count = 0;
    for(num_t i = 0; i < size; i++)
    {
        prime_count += primes[i];
    }
    return prime_count;
}