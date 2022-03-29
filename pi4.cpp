#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>


/*
threads_num=8
<time.h> time=0.845922
 <omp.h> time=0.110403
Wartosc liczby PI wynosi  3.141592653590

threads_num=4
<time.h> time=0.440836
 <omp.h> time=0.111723
Wartosc liczby PI wynosi  3.141592653590

MEMSHIFT eksperyment
<time.h> time=4.301967
 <omp.h> time=2.151685

threads_num=2
<time.h> time=0.437099
 <omp.h> time=0.218620
Wartosc liczby PI wynosi  3.141592653590
*/

// a) liczba procesorow logicznych
#define THREADS_LOGIC 8
// b) liczba procesorow fizycznych
#define THREADS_PHYSICAL 4
// c) polowa liczby procesorow fizycznych
#define THREADS_HALF THREADS_PHYSICAL / 2

#define THREADS_POLICY THREADS_PHYSICAL

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

#define NUM_STEPS 100000000
double step;

int main(int argc, char *argv[]) {
  int threads_num = THREADS_POLICY;
  printf("threads_num=%d\n", threads_num);
  omp_set_num_threads(threads_num);

  double pi, sum = 0.0;

  bench_t bench;
  bench.T1();

#pragma omp parallel default(none) shared(sum)
  {
    double step2 = (1. / NUM_STEPS) * (1. / NUM_STEPS);
    double priv_sum = 0.0;

#pragma omp for schedule(guided)
    for (int i = 0; i < NUM_STEPS; i++)
      priv_sum += 4.0 / (1. + (i + .5) * (i + .5) * (step2));

#pragma omp atomic
    sum += priv_sum;
  }

  double step = 1. / NUM_STEPS;
  pi = sum * step;

  bench.T2();
  bench.print();

  printf("Wartosc liczby PI wynosi %15.12f\n", pi);

  return 0;
}