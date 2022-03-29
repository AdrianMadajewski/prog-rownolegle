#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Q: ile wynosi "cache line"/L1?

// A: z eksperymentu wynika ze MEMSHIFT=7 (bo czasy sa takie jak w PI4)
//    a wiec dlugosc linii wynosi [8 * 8 bajtow = 64] na dane
//    (co jest zgodne z moja architektura komputera)

/*
threads_num=2

MEMSHIFT=0
  0 -> vsum[ 0]   1 -> vsum[ 1] 
<time.h> time=4.303696
 <omp.h> time=2.156671

MEMSHIFT=1
  0 -> vsum[ 0]   1 -> vsum[ 2] 
<time.h> time=4.293185
 <omp.h> time=2.147952

MEMSHIFT=2
  0 -> vsum[ 0]   1 -> vsum[ 3] 
<time.h> time=4.293470
 <omp.h> time=2.148403

MEMSHIFT=3
  0 -> vsum[ 0]   1 -> vsum[ 4] 
<time.h> time=4.292647
 <omp.h> time=2.150212

MEMSHIFT=4
  0 -> vsum[ 0]   1 -> vsum[ 5] 
<time.h> time=4.297119
 <omp.h> time=2.148709

MEMSHIFT=5
  0 -> vsum[ 0]   1 -> vsum[ 6] 
<time.h> time=4.295603
 <omp.h> time=2.150257

MEMSHIFT=6
  0 -> vsum[ 0]   1 -> vsum[ 7] 
<time.h> time=4.293722
 <omp.h> time=2.148626

MEMSHIFT=7
  0 -> vsum[ 0]   1 -> vsum[ 8] 
<time.h> time=4.301967
 <omp.h> time=2.151685

MEMSHIFT=8
  0 -> vsum[ 0]   1 -> vsum[ 9] 
<time.h> time=4.293949
 <omp.h> time=2.148078

MEMSHIFT=9
  0 -> vsum[ 0]   1 -> vsum[10] 
<time.h> time=4.294100
 <omp.h> time=2.148266

MEMSHIFT=10
  0 -> vsum[ 0]   1 -> vsum[11] 
<time.h> time=4.294153
 <omp.h> time=2.148081

MEMSHIFT=11
  0 -> vsum[ 0]   1 -> vsum[12] 
<time.h> time=4.292602
 <omp.h> time=2.147205

MEMSHIFT=12
  0 -> vsum[ 0]   1 -> vsum[13] 
<time.h> time=4.292139
 <omp.h> time=2.147963

MEMSHIFT=13
  0 -> vsum[ 0]   1 -> vsum[14] 
<time.h> time=4.297373
 <omp.h> time=2.149790

MEMSHIFT=14
  0 -> vsum[ 0]   1 -> vsum[15] 
<time.h> time=4.354864
 <omp.h> time=2.180438

MEMSHIFT=15
  0 -> vsum[ 0]   1 -> vsum[16] 
<time.h> time=4.359618
 <omp.h> time=2.180344

*/

#define THREADS_POLICY 2

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

#define MEMSHIFT 24
volatile double vsum[THREADS_POLICY * (MEMSHIFT + 1)] = {0};

int main(int argc, char *argv[])
{
    int threads_num = THREADS_POLICY;
    printf("threads_num=%d\n", threads_num);
    omp_set_num_threads(threads_num);

    for (int memshift = 0; memshift < MEMSHIFT; memshift++)
    {
        printf("\n\033[92mMEMSHIFT=%d\033[m\n", memshift);
        for (int i = 0; i < THREADS_POLICY; i++)
            printf(" %2d -> vsum[%2d] ", i, i + i * memshift);
        printf("\n");

        double pi, sum = 0.0;

        for (int i = 0; i < THREADS_POLICY * (MEMSHIFT + 1); i++)
            vsum[i] = 0;

        bench_t bench;
        bench.T1();

#define NUM_STEPS 1000000000

#pragma omp parallel default(none) shared(vsum, memshift)
        {
            double step2 = (1. / NUM_STEPS) * (1. / NUM_STEPS);
            int idx = omp_get_thread_num();

#pragma omp for schedule(static, 1) nowait
            for (int i = 0; i < NUM_STEPS; i++)
                vsum[idx + idx * memshift] +=
                    4.0 / (1. + (i + .5) * (i + .5) * (step2));
        }

        double step = 1. / NUM_STEPS;
        for (int i = 0; i < THREADS_POLICY * (MEMSHIFT + 1); i++)
            sum += vsum[i];
        pi = sum * step;

        bench.T2();
        bench.print();
    }

    return 0;
}