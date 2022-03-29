#include <stdio.h>
#include <time.h>
#include <omp.h>

// g++ -fopenmp pi1.cpp -o pi1 -O3

long long num_steps = 1000000000;
double step;

// Q: dlaczego linuxowy czas uzycia procesor rozni sie od (wall clock)
//    i wyjasnic przyczyne.

// A: wall clock to czas jaki minal od punktu p1 do p2 (real clock),
//    natomiast clock() time (czyli system-cpu time) to
//    czas ktory program spedzil na procesorze (kernel code).
//    Wiec jak cos wywlaszczylo na 5 sec nasz program,
//    to ten czas nie bedzie liczony.

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
		printf("<time.h> time=%f\n", ((double)(C2 - C1) / CLOCKS_PER_SEC));
		printf(" <omp.h> time=%f\n", ((double)(O2 - O1)));
	}
};

// Bez zrownoleglenia
int main(int argc, char *argv[])
{
	double x, pi, sum = 0.0;
	int i;
	step = 1. / (double)num_steps;

	bench_t bench;
	bench.T1();

	for (i = 0; i < num_steps; i++)
	{
		x = (i + .5) * step;
		sum = sum + 4.0 / (1. + x * x);
	}

	pi = sum * step;

	bench.T2();
	bench.print();

	printf("Wartosc liczby PI wynosi %15.12f\n", pi);

	return 0;
}
