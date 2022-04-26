#include <stdio.h>
#include <omp.h>
#include <time.h>

// a) liczba procesorow logicznych
#define THREADS_LOGIC 8
// b) liczba procesorow fizycznych
#define THREADS_PHYSICAL 4
// c) polowa liczby procesorow fizycznych
#define THREADS_HALF THREADS_PHYSICAL / 2

// CURRENT THREAD POLICY
#define THREADS_POLICY THREADS_PHYSICAL

#define NUM_STEPS 1000000000

struct bench_t
{
	clock_t C1, C2;
	double O1, O2;

	void T1()
	{
		C1 = clock();
		O1 = omp_get_wtime();
	}

	void T2()
	{
		C2 = clock();
		O2 = omp_get_wtime();
	}

	void print()
	{   
		printf("Czas procesorów przetwarzania równoleglego: %f sekund\n", ((double)(C2 - C1) / CLOCKS_PER_SEC));
		printf("Czas trwania obliczen rownoleglych - wallclock: %f sekund\n", ((double)(O2 - O1)));
	}
};