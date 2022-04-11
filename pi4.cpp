#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>
#include "bench.cpp"

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

int main(int argc, char *argv[])
{
  int threads_num = THREADS_POLICY;
  omp_set_num_threads(threads_num);

  double pi, sum = 0.0;

  bench_t bench;
  bench.T1();

#pragma omp parallel default(none) shared(sum)
  {
    double step = (1. / NUM_STEPS);
    double priv_sum = 0.0;

#pragma omp for
    for (int i = 0; i < NUM_STEPS; i++)
    {
      double x = (i + .5) * step;
      priv_sum += 4.0 / (1. + x * x);
    }

#pragma omp atomic
    sum += priv_sum;
  }

  double step = 1. / (double)NUM_STEPS;
  pi = sum * step;

  bench.T2();
  bench.print();

  printf("Liczba dostępnych procesorów: %d\n", THREADS_POLICY);
  printf("Wartosc liczby PI wynosi %15.12f\n", pi);

  return 0;
}