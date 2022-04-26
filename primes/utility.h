#ifndef BENCH_H
#define BENCH_H

#define THREADS 1

typedef unsigned long long num_t;
typedef char array_t;

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct 
{
    clock_t clockStart, clockEnd;
	double wallClockStart, wallClockEnd;
} bench_t;

void printPrimes(array_t *primes, num_t size, num_t shift);
num_t countPrimes(array_t *primes, num_t size);

#endif