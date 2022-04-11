#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>
#include "bench.cpp"

/*
threads_num=8
<time.h> time=0.872258
 <omp.h> time=0.111787
Wartosc liczby PI wynosi  0.497419978187

threads_num=4
<time.h> time=0.436874
 <omp.h> time=0.111364
Wartosc liczby PI wynosi  0.979914652507

threads_num=2
<time.h> time=0.434154
 <omp.h> time=0.218280
Wartosc liczby PI wynosi  1.854590436003
*/

int main(int argc, char *argv[])
{
    int threads_num = THREADS_POLICY;
    omp_set_num_threads(threads_num);

    double x, pi, step, sum = 0.0;
    int i;
    step = 1. / (double)NUM_STEPS;

    bench_t bench;
    bench.T1();

// dla jasnosci wypisalem w klazuri shared te zmienne
#pragma omp parallel default(none) shared(x, sum, i, step)
    {
#pragma omp for schedule(guided)
        for (i = 0; i < NUM_STEPS; i++)
        {
            x = (i + .5) * step;
            sum += 4.0 / (1. + x * x);
        }
    }

    // obliczenia beda nieprawidlowe
    pi = sum * step;

    bench.T2();
    bench.print();

    printf("Liczba dostępnych procesorów: %d\n", THREADS_POLICY);
    printf("Wartosc liczby PI wynosi %15.12f\n", pi);

    return 0;
}