#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bench.cpp"

/*
threads_num=8
<time.h> time=0.847507
 <omp.h> time=0.110941
Wartosc liczby PI wynosi  3.

threads_num=4
<time.h> time=0.436716
 <omp.h> time=0.110399
Wartosc liczby PI wynosi  3.141592653590

threads_num=2
<time.h> time=0.431820
 <omp.h> time=0.219384
Wartosc liczby PI wynosi  3.141592653590
*/

int main(int argc, char *argv[])
{
    int threads_num = THREADS_POLICY;
    omp_set_num_threads(threads_num);

    double pi, sum = 0.0;

    bench_t bench;
    bench.T1();

    double step = 1. / NUM_STEPS;

#pragma omp parallel for reduction(+ \
                                   : sum)
    for (int i = 0; i < NUM_STEPS; i++)
    {
        double x = (i + .5) * step;
        sum += 4.0 / (1. + x * x);
    }

    pi = sum * step;

    bench.T2();
    bench.print();

    printf("Liczba dostępnych procesorów: %d\n", THREADS_POLICY);
    printf("Wartosc liczby PI wynosi %15.12f\n", pi);

    return 0;
}