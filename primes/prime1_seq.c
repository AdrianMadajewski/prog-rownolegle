#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

#include "utility.h"

// gcc prime1_seq.c bench.c -o prime1 -fopenmp -O3 -D DEBUG

int isPrime(num_t number)
{
    for(int i = 2; i * i <= number; i++) 
    {
        if(number % i == 0)
            return 0;
    }
    return 1;
}

num_t primesInRange(array_t *primes, num_t size, num_t start, num_t end)
{
    num_t prime_count = 0;
    for(num_t i = start; i <= end; i++)
    {
        if(isPrime(i))
        {
            primes[size - end + i] = 1;
            prime_count++;
        }
    }
    return prime_count;
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("Usage: %s start_number end_number\n", argv[0]);
        return 1;
    }

    omp_set_num_threads(THREADS);

    num_t start = strtoull(argv[1], NULL, 0);
    num_t end = strtoull(argv[2], NULL, 0);
    num_t size = end - start + 1;

    array_t *primes = (array_t*) malloc(sizeof(array_t) * size);
    memset(primes, 0, sizeof(primes));

    #ifdef TIME
        bench_t bench;
        bench.clockStart = clock();
        bench.wallClockStart = omp_get_wtime();
    #endif

    num_t prime_count = primesInRange(primes, size, start, end);
    
    #ifdef DEBUG
        printPrimes(primes, size, start);
    #endif

     #ifdef TIME
        bench.clockEnd = clock();
        bench.wallClockEnd = omp_get_wtime();

        printf("Czas procesorów: %f sekund \n", ((double)(bench.clockEnd - bench.clockStart) / CLOCKS_PER_SEC));
        printf("Czas trwania obliczen - wallclock: %f sekund \n", bench.wallClockEnd - bench.wallClockStart);
    #endif

    printf("Primes found: %llu\n", prime_count);

    free(primes);
    return 0;
}
