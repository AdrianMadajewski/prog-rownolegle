#include <cstdio>
#include <ctime>
#include <iostream>
#include <omp.h>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cmath>

using namespace std;

#define BLOCK_SIZE 64 * 1024
#define MAX 100000
#define MIN 2 
#define THREADS 4

void show_results(vector<int>& arr, int min = MIN, int max = MAX, bool reverse = true, bool showAll = true)
{
	int counter = 0;
	for (int i = 0; i < arr.size(); i++)
	{
		if (reverse)
		{
			if (arr[i] != -1)
			{
				if (showAll)
				{
					cout << arr[i];
					if (counter % 10 == 9)
						cout << "\n";
					else
						cout << " ";
				}
				counter++;
			}
		}
		else
		{
			if (arr[i] == -1)
			{
				if (showAll)
				{
					cout << i + min;
					if (counter % 10 == 9)
						cout << "\n";
					else
						cout << " ";
				}
				counter++;
			}
		}
	}
	if (showAll)
		cout << "\n\n";

	cout << "Dla przedzialu od " << min << " do " << max << " znaleziono " << counter << " liczb pierwszych.\n\n";
}

vector<int> sieve_in_range(int min, int max)
{
	int vectorSize = (max - min + 1) / 2; // Ustalenie rozmiaru vectora wykreśleń na połowę rozmiaru, ze względu na nieobecność liczb parzystych.
	int endBound = floor(sqrt(max)) + 1; // Sprawdzane będzie do pierwiastka z przedziału.

	vector<int> isPrime(vectorSize, 1);

	for (int i = 3; i < endBound; i += 2) // Iterowanie pomijające liczby parzyste.
	{
		int startNumber = ((min + i - 1) / i) * i; // Ustalenie pierwszej liczby na pierwszą wielokrotność 'i' większą od minimum (o ile jest większa niż i * i).
		if (startNumber < i * i)
			startNumber = i * i; // W przypadku, gdy początkowa liczba jest mniejsza niż i * i, przypisanie tej wartości. Skoro aby wyznaczyć liczby pierwsze dla przydziału wystarczy wykreślić wielokrotności do pierwiastka z MAX, podobnie jest z każdą liczbą w tym przedziale, ponieważ ona równieć jest wartością maksymalną pewnego przedziału.
		if (startNumber % 2 == 0)
			startNumber += i; // Gdy początkowa liczba jest parzysta - wyznaczenie kolejnej wielokrotności tej liczby tak, aby wykreślać kolejne nieparzyste wielokrotności.

		for (int j = startNumber; j < max; j += 2 * i)
			isPrime[(j - min) / 2] = 0; // Zaznaczanie, że dana liczba nie jest pierwsza (krok co 2 * i, przez co pomija się liczby parzyste i ogranicza wielkość vectora).
	}

	vector<int> primes;

	if (min <= 2)
		primes.push_back(2); // Ręczne dodanie liczby 2, która mogła zostać wcześniej wykreślona.

	for (int i = 0; i < vectorSize; i++)
	{
		if (isPrime[i] == 1)
		{
			primes.push_back(2 * i + min + 1); // Dodawanie kolejnych liczb pierwszych do vectora.
		}
	}

	return primes; // Zwrócenie vectora liczb pierwszych w danym przedziale.
}


void domain_sieve(vector<int>& arr, int threads)
{
	omp_set_num_threads(threads);
	int arrSize = arr.size();
	int min = arr[0];
	int max = arr[arrSize - 1];

	vector<int> primes = sieve_in_range(2, floor(sqrt(max)) + 1); // Wyznaczenie liczb pierwszych z przedziału (2, pierwiastek z maximum przedziału).
	int primesSize = primes.size(); // Wyznaczenie liczby liczb pierwszych do pierwiastka z maximum, w celu uniknięcia nadmiarowych obliczeń w pętli.

#pragma omp parallel
	{
#pragma omp for nowait
		for (int i = 0; i <= max - min; i += BLOCK_SIZE) // Przetwarzanie blokowe, co BLOCK_SIZE, w celu trzymania w pamięci cache optymalnej ilości danych.
		{
			int minimum = min; // Przypisanie wartości do zmiennych lokalnych, ze względu na częste odwoływanie się do nich. Przy próbie dodania tych linii po #pragma omp parallel a przed #pragma omp for występowały błędy dla niektórych przedziałów. Z tego względu zostało to dodane już w pętli, jednak nie wpływa to znacząco na przetwarzanie.
			int maximum = max;

			for (int j = 0; j < primesSize; j++) // Pętla po wszystkich liczbach pierwszych do pierwiastka z maximum.
			{
				int currPrime = primes[j];
				int beg = minimum + i; // Wyznaczenie liczby równej początku przetwarzania w danym bloku.
				int t = beg / currPrime + (beg % currPrime == 0 ? 0 : 1); // Wyznaczenie od której wielokrotności liczby pierwszej zacząć. Jeśli początek jest od razu wielokrotnością, zaczyna się od niego, w przeciwnym wypadku należy wyznaczyć kolejną wielokrotność, aby była większa niż minimum.
				if (t < 2)
					t = 2; // Zabezpieczenie przed wykreśleniem jako wielokrotność samej liczby pierwszej.

				int end = minimum + i + BLOCK_SIZE; // Wyznaczenie liczby równej końca przetwarzania w danym bloku.
				if (end > maximum)
					end = maximum; // Zabezpieczenie przed sytuacją, gdyby przetwarzanie zaszło powyżej maximum przedziału.

				while (t * currPrime <= end)
				{
					arr[t * currPrime - minimum] = -1; // Zaznaczanie, że dana liczba nie jest liczbą pierwszą.
					t++;
				}
			}
		}
	}
}

void set_up_vector(vector<int> &arr, int min, int max)
{
    for(int i = min; i <= max; ++i)
        arr.emplace_back(i);
}

int main()
{
    std::vector<int> array;
    set_up_vector(array, MIN, MAX);

    domain_sieve(array, THREADS);

    show_results(array);

    return 0;
}