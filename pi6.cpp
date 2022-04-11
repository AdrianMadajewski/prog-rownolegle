#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bench.cpp"

/*
threads_num=8
<time.h> time=0.881398
 <omp.h> time=0.115311
Wartosc liczby PI wynosi  3.141592653590

threads_num=4
<time.h> time=0.436580
 <omp.h> time=0.110385
Wartosc liczby PI wynosi  3.141592653590

threads_num=2
<time.h> time=0.433084
 <omp.h> time=0.219744
Wartosc liczby PI wynosi  3.141592653590

*/

int main(int argc, char *argv[])
{
    int threads_num = THREADS_POLICY;
    omp_set_num_threads(threads_num);

    double pi, sum = 0.0;
    volatile double partial_sums[THREADS_POLICY] = {0};
    double step = 1. / (double)NUM_STEPS;

    bench_t bench;
    bench.T1();

#pragma omp parallel
    {
        int id = omp_get_thread_num();
#pragma omp for
        for (int i = 0; i < NUM_STEPS; i++)
        {
            double x = (i + .5) * step;
            partial_sums[id] += 4.0 / (1. + x * x);
        }
    }
    for (int i = 0; i < THREADS_POLICY; i++)
    {
        sum += partial_sums[i];
    }

    pi = sum * step;

    bench.T2();
    bench.print();
    printf("Liczba dostępnych procesorów: %d\n", THREADS_POLICY);
    printf("Wartosc liczby PI wynosi %15.12f\n", pi);

    return 0;
}