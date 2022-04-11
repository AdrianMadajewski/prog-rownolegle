#include <stdio.h>
#include <time.h>
#include <omp.h>
#include "bench.cpp"

// g++ -fopenmp pi1.cpp -o pi1 -O3

// Q: dlaczego linuxowy czas uzycia procesor rozni sie od (wall clock)
//    i wyjasnic przyczyne.

// A: wall clock to czas jaki minal od punktu p1 do p2 (real clock),
//    natomiast clock() time (czyli system-cpu time) to
//    czas ktory program spedzil na procesorze (kernel code).
//    Wiec jak cos wywlaszczylo na 5 sec nasz program,
//    to ten czas nie bedzie liczony.


// Bez zrownoleglenia
int main(int argc, char *argv[])
{
	double x, pi, step, sum = 0.0;
	int i;
	step = 1. / (double)NUM_STEPS;

	bench_t bench;
	bench.T1();

	for (i = 0; i < NUM_STEPS; i++)
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
