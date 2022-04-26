#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>
#include "bench.cpp"

int main(int argc, char *argv[])
{
  int threads_num = THREADS_POLICY;
  omp_set_num_threads(threads_num);

  double pi, sum = 0.0;
  double step = 1. / (double)NUM_STEPS;

  bench_t bench;
  bench.T1();

#pragma omp parallel default(none) shared(sum, step)
  {
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

  
  pi = sum * step;

  bench.T2();
  bench.print();

  printf("Liczba dostępnych procesorów: %d\n", THREADS_POLICY);
  printf("Wartosc liczby PI wynosi %15.12f\n", pi);

  return 0;
}