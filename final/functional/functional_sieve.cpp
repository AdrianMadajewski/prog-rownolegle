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

#define FUNCTION_BLOCK_SIZE 64 * 1024 // Wielkość bloku przetwarzania dla algorytmu sita funkcyjnego.
#define FUNCTION_PRIMES_BLOCK_SIZE 200 // Wielkość bloku przetwarzania liczb pierwszych w algorytmie sita funkcyjnego. Liczba 200 dobrana nie dla najszybszego przetwarzania, lecz w celu zapewnienia, że dla MAX = 135000000, każdy wątek otrzyma część pracy.

void ShowResults(vector<int>& arr, int min = MIN, int max = MAX, bool reverse = true, bool showAll = true)
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

void SetUpArray(vector<int>& arr, int min, int max)
{
	 for(int i = min; i <= max; ++i)
        arr.emplace_back(i);
}

vector<int> SieveRange(int min, int max)
{
	int vecSize = (max - min + 1) / 2; // Ustalenie rozmiaru vectora wykreśleń na połowę rozmiaru, ze względu na nieobecność liczb parzystych.
	int endFor = floor(sqrt(max)) + 1; // Sprawdzane będzie do pierwiastka z przedziału.

	vector<int> isPrime(vecSize, 1);

	for (int i = 3; i < endFor; i += 2) // Iterowanie pomijające liczby parzyste.
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
	for (int i = 0; i < vecSize; i++)
	{
		if (isPrime[i] == 1)
		{
			primes.push_back(2 * i + min + 1); // Dodawanie kolejnych liczb pierwszych do vectora.
		}
	}

	return primes; // Zwrócenie vectora liczb pierwszych w danym przedziale.
}

double FunctionSieve(vector<int>& arr, int threads)
{
	omp_set_num_threads(threads);
	double start, stop;
	start = omp_get_wtime();

	int arrSize = arr.size();
	int min = arr[0];
	int max = arr[arrSize - 1];

	vector<int> primes = SieveRange(2, floor(sqrt(max)) + 1); // Wyznaczenie liczb pierwszych z przedziału (2, pierwiastek z maximum przedziału).
	int primesSize = primes.size(); // Wyznaczenie liczby liczb pierwszych do pierwiastka z maximum, w celu uniknięcia nadmiarowych obliczeń w pętli.

#pragma omp parallel
	{
#pragma omp for schedule(dynamic) nowait
		for (int p = 0; p < primesSize; p += FUNCTION_PRIMES_BLOCK_SIZE) // Przetwarzanie liczb pierwszych w blokach co FUNCTION_PRIMES_BLOCK_SIZE.
		{
			int minimum = min; // Przypisanie wartości do zmiennych lokalnych, ze względu na częste odwoływanie się do nich. Przy próbie dodania tych linii po #pragma omp parallel a przed #pragma omp for występowały błędy dla niektórych przedziałów. Z tego względu zostało to dodane już w pętli, jednak nie wpływa to znacząco na przetwarzanie.
			int maximum = max;
			for (int i = 0; i < arrSize; i += FUNCTION_BLOCK_SIZE) // Iterowanie po tablicy wykreśleń blokowo z krokiem co FUNCTION_BLOCK_SIZE.
			{
				int beg = minimum + i; // Wyznaczenie liczby równej początku przetwarzania w danym bloku.
				int end = minimum + i + FUNCTION_BLOCK_SIZE; // Wyznaczenie liczby równej końcu przetwarzania w danym bloku.
				if (end > maximum)
					end = maximum; // Zabezpieczenie przed sytuacją, gdyby przetwarzanie zaszło powyżej maximum przedziału.

				int begPrimes = p; // Wyznaczenie pierwszej przetwarzanej w danym bloku liczby pierwszej.
				int endPrimes = p + FUNCTION_PRIMES_BLOCK_SIZE; // Wyznaczenie ostatniej przetwarzanej w danym bloku liczby pierwszej.
				if (endPrimes >= primesSize)
					endPrimes = primesSize; // Zabezpieczenie przed przetwarzaniem liczby pierwszej większej od maksymalnej z przedziału.

				for (int j = begPrimes; j < endPrimes; j++) // Iterowanie po kolejnych liczbach pierwszych w danym bloku.
				{
					int currPrime = primes[j];
					int t = floor(beg / currPrime) + (beg % currPrime == 0 ? 0 : 1); // Wyznaczenie najmniejszej wielokrotności liczby pierwszej, która jest jednocześnie większa od początku przedziału tablicy wykreśleń.
					if (t < 2)
						t = 2; // Zabezpieczenie przed wykreśleniem jako wielokrotność samej liczby pierwszej.

					while (t * currPrime <= end)
					{
						arr[t * currPrime - minimum] = -1; // Zaznaczanie, że dana liczba nie jest liczbą pierwszą.
						t++;
					}
				}
			}
		}
	}

	stop = omp_get_wtime();

	return (stop - start) * 1000.; // czas w ms
}

int main()
{
    std::vector<int> array;
    SetUpArray(array, MIN, MAX);

    std::cout << FunctionSieve(array, THREADS) << std::endl;

    ShowResults(array);

    return 0;
}