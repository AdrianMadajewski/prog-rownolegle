#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>

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


// a) liczba procesorow logicznych
#define THREADS_LOGIC 8
// b) liczba procesorow fizycznych
#define THREADS_PHYSICAL 4
// c) polowa liczby procesorow fizycznych
#define THREADS_HALF THREADS_PHYSICAL / 2

#define THREADS_POLICY THREADS_HALF

struct bench_t
{
    clock_t C1, C2;
    double O1, O2;

    void T1()
    {
        this->C1 = clock();
        this->O1 = omp_get_wtime();
    }

    void T2()
    {
        this->C2 = clock();
        this->O2 = omp_get_wtime();
    }

    void print()
    {
        printf("<time.h> time=%f\n", ((double)(C2 - C1) / CLOCKS_PER_SEC));
        printf(" <omp.h> time=%f\n", ((double)(O2 - O1)));
    }
};

long long num_steps = 100000000;
double step;

int main(int argc, char *argv[])
{
    int threads_num = THREADS_POLICY;
    printf("threads_num=%d\n", threads_num);
    omp_set_num_threads(threads_num);

    double x, pi, sum = 0.0;
    int i;
    step = 1. / (double)num_steps;

    bench_t bench;
    bench.T1();

    // dla jasnosci wypisalem w klazuri shared te zmienne
#pragma omp parallel default(none) shared(x, sum, i, step, num_steps)
    {
#pragma omp for schedule(guided)
        for (i = 0; i < num_steps; i++)
        {
            x = (i + .5) * step;
            sum = sum + 4.0 / (1. + x * x);
        }
    }

    // obliczenia beda nieprawidlowe
    pi = sum * step;

    bench.T2();
    bench.print();

    printf("Wartosc liczby PI wynosi %15.12f\n", pi);

    return 0;
}