#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include "bench.cpp"

#define THREADS 2
#define MEMSHIFT 25

int main(int argc, char *argv[])
{
    double sum = 0.0;
    volatile double partial_sums[MEMSHIFT * THREADS] = {0};
    double step = 1. / (double)(NUM_STEPS);

    for (int i = 0; i < THREADS * MEMSHIFT; i++)
    {
        // printf("Iteracja: %d\n", i + 1);

        bench_t bench;
        bench.T1();

        #pragma omp parallel num_threads(THREADS)
        {
            int id = omp_get_thread_num();
            int access_address = (id == 0 ? i : i + 1);

            #pragma omp for 
            for(int j = 0; j < NUM_STEPS; j++)
            {
                double x = (i + .5) * step;
                partial_sums[access_address] += 4.0 / (1. + x * x);
            }
            #pragma omp atomic
            sum += partial_sums[access_address];
        }
        bench.T2();
        // bench.print();
        printf("%d\t%f\t%f\n", i + 1, 
            ((double)(bench.C2 - bench.C1) / CLOCKS_PER_SEC),
            (double)(bench.O2 - bench.O1)
            );
    }

    return 0;
}