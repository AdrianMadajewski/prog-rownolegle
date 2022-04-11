#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>
#include "bench.cpp"

// Q: w jaki sposób można na poziomie wątku zapewnić atomowość
//    (niepodzielność) uaktualnienia zmiennej współdzielonej w systemie.

// A: w OpenMP mozemy uzyc "atomic"/"reduction" lub stworzyc "critical" section
//    jednak z doswiadczenia stworzenie "#pragma omp critical"
//    jest duzo bardziej kosztowne niz wskazanie "#pragma omp atomic"
//    na dana pojedyncza operacje
//    ---
//    jesli nie chcemy korzystac z takich "wbudowanych" podejsc
//    mozemy zastosowac vectoryzacje (ten pomysl przejawia sie w PI6)
//    trzeba jednak pamietac o false sharingu

/*
threads_num=8
<time.h> time=141.339692
 <omp.h> time=17.948450
Wartosc liczby PI wynosi  3.141592653591

threads_num=4
<time.h> time=50.136117
 <omp.h> time=12.541923
Wartosc liczby PI wynosi  3.141592653590

threads_num=2
<time.h> time=11.375469
 <omp.h> time=5.688302
Wartosc liczby PI wynosi  3.141592653590

3. Zrealizować ciąg dostępów (odczyt, zapis) do współdzielonej sumy w sposób
niepodzielny:

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
        double step = 1. / (double)NUM_STEPS;

#pragma omp for schedule(guided)
        for (int i = 0; i < NUM_STEPS; i++)
        {
            double x = (i + .5) * step;
#pragma omp atomic
            sum += 4.0 / (1. + x * x);
        }
    }

    double step = 1. / (double)NUM_STEPS;
    pi = sum * step;

    bench.T2();
    bench.print();

    printf("Liczba dostępnych procesorów: %d\n", THREADS_POLICY);
    printf("Wartosc liczby PI wynosi %15.12f\n", pi);

    return 0;
}