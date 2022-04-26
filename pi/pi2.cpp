#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>
#include "bench.cpp"

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