#include <cstdio>
#include <ctime>
#include <iostream>
#include <omp.h>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cmath>

#define MAX 50 // Maksymalna wartość przedziału.
#define MAX_HALF MAX / 2 // Połowa przedziału.
#define SINGLE1_OR_ALL2 1 // Ustalenie, czy program ma dotyczyć jednego przedziału, czy wszystkich.
#define SINGLE_CHOOSE_RANGE123 1 // W przypadku jednego przedziału, ustalenie który z nich wybrać.
#define FIRST1_OR_SIEVE2 1 // Ustalenie czy program ma działać w metodzie podstawowej, czy sitem.
#define DOMAIN1_OR_FUNCTION2 2 // Ustalenie sposobu przetwarzania dla sita równoległego.
#define BLOCK_SIZE 64 * 1024 // Wielkość bloku przetwarzania dla algorytmu sita sekwencyjnego oraz domenowego.
#define FUNCTION_BLOCK_SIZE 64 * 1024 // Wielkość bloku przetwarzania dla algorytmu sita funkcyjnego.
#define FUNCTION_PRIMES_BLOCK_SIZE 200 // Wielkość bloku przetwarzania liczb pierwszych w algorytmie sita funkcyjnego. Liczba 200 dobrana nie dla najszybszego przetwarzania, lecz w celu zapewnienia, że dla MAX = 135000000, każdy wątek otrzyma część pracy.
#define SHOW_ALL_PRIME_NUMBERS true // Definiowanie, czy domyślnie wyświetlać wszystkie liczby pierwsze wyznaczone w danym przedziale (w razie potrzeby, można to kontrolować przy wywoływaniu ShowResults osobno dla każdego vectora.

using namespace std;

void ShowResults(vector<int>& arr, int min = 2, int max = MAX, bool reverse = true, bool showAll = SHOW_ALL_PRIME_NUMBERS)
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
					if (++counter == 10)
						cout << "\n";
					else
						cout << " ";
				}
				counter %= 10;
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
				counter %= 10;
			}
		}
	}
	if (showAll)
		cout << "\n\n";

	cout << "Dla przedzialu od " << min << " do " << max << " znaleziono " << counter << " liczb pierwszych.\n\n";
}

void SetUpArray(vector<int>& arr, int version)
{
	arr.clear();

	if (version == 1)
		for (int i = 2; i <= MAX; i++)
			arr.push_back(i);
	else if (version == 2)
		for (int i = MAX_HALF; i <= MAX; i++)
			arr.push_back(i);
	else if (version == 3)
		for (int i = 2; i <= MAX_HALF; i++)
			arr.push_back(i);
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

double SeqPrime(vector<int>& arr)
{
	clock_t start, stop;
	start = clock();

	for (int i = 0; i < arr.size(); i++)
	{
		int number = arr[i]; // Przypisanie do 'number' sprawdzanej wartości, aby możliwie mało razy odwoływać się bezpośrednio do vectora (przydatne przy algorytmie równoległym).

		if (number % 2 == 0 && number != 2) // Pomijanie w przetwarzaniu liczb parzystych
		{
			arr[i] = -1;
			continue;
		}

		int sq = floor(sqrt(number)) + 1; // Przypisanie do zmiennej wartości pierwiastka ze sprawdzanej liczby w celu zminimalizowania powtarzających się obliczeń w pętli. Pozwoliło to na przyspieszenie algorytmu o 8-12%.
		bool divided = false;
		for (int j = 3; j < sq && !divided; j += 2) // W trakcie wykonywania pętli, za każdym razem sprawdzane jest, czy dana liczba została już wcześniej podzielona. Jeśli tak - program wychodzi z pętli. Pomijane są również liczby parzyste.
		{
			if (number % j == 0) // Sprawdzanie, czy dana liczba 'number' jest podzielna przez 'j'.
				divided = true;
		}
		if (divided)
			arr[i] = -1; // Oznaczenie, że dana liczba nie jest liczbą pierwszą.
	}

	stop = clock();

	return (stop - start) / 1000.;
}

double ParPrime(vector<int>& arr, const int threads)
{
	omp_set_num_threads(threads);
	clock_t start, stop;
	start = clock();

#pragma omp parallel
	{
#pragma omp for schedule(dynamic, 500) nowait
		for (int i = 0; i < arr.size(); i++)
		{
			int number = arr[i]; // Przypisanie do 'number' sprawdzanej wartości, aby możliwie mało razy odwoływać się bezpośrednio do vectora, co zmniejszy szansę na unieważnienie linii pamięci podręcznej.
		
			if (number % 2 == 0 && number != 2) // Pomijanie w przetwarzaniu liczb parzystych
			{
				arr[i] = -1;
				continue;
			}
		
			int sq = floor(sqrt(number)) + 1; // Przypisanie do zmiennej wartości pierwiastka ze sprawdzanej liczby w celu zminimalizowania powtarzających się obliczeń w pętli. Pozwoliło to na przyspieszenie algorytmu o 8-12%.
			bool divided = false;
			for (int j = 3; j < sq && !divided; j += 2) // W trakcie wykonywania pętli, za każdym razem sprawdzane jest, czy dana liczba została już wcześniej podzielona. Jeśli tak - program wychodzi z pętli. Pomijane są również liczby parzyste.
			{
				if (number % j == 0) // Sprawdzanie, czy dana liczba 'number' jest podzielna przez 'j'.
					divided = true;
			}
			if (divided)
				arr[i] = -1; // Oznaczenie, że dana liczba nie jest liczbą pierwszą.
		}
		cout << omp_get_thread_num() << " ";
	}

	stop = clock();

	return (stop - start) / 1000.;
}

double SeqPrimeImproved(vector<int>& arr)
{
	clock_t start, stop;
	start = clock();

	vector<int> primes = SieveRange(2, floor(sqrt(arr[arr.size() - 1])) + 1); // Wyznaczenie liczb pierwszych do pierwiastka z wartości maksymalnej
	int primesSize = primes.size();

	for (int i = 0; i < arr.size(); i++)
	{
		int number = arr[i]; // Przypisanie do 'number' sprawdzanej wartości, aby możliwie mało razy odwoływać się bezpośrednio do vectora (przydatne przy algorytmie równoległym).

		if (number % 2 == 0 && number != 2) // Pomijanie w przetwarzaniu liczb parzystych
		{
			arr[i] = -1;
			continue;
		}

		int sq = floor(sqrt(number)) + 1; // Przypisanie do zmiennej wartości pierwiastka ze sprawdzanej liczby w celu zminimalizowania powtarzających się obliczeń w pętli. Pozwoliło to na przyspieszenie algorytmu o 8-12%.
		bool divided = false;
		for (int j = 0; j < primesSize && !divided; j++) // W trakcie wykonywania pętli, za każdym razem sprawdzane jest, czy dana liczba została już wcześniej podzielona. Jeśli tak - program wychodzi z pętli.
		{
			int t = primes[j];
			if (number % t == 0 && number != t) // Sprawdzanie, czy dana liczba 'number' jest podzielna przez liczbę pierwszą z vectora.
				divided = true;
		}
		if (divided)
			arr[i] = -1; // Oznaczenie, że dana liczba nie jest liczbą pierwszą.
	}

	stop = clock();

	return (stop - start) / 1000.;
}

double ParPrimeImproved(vector<int>& arr, const int threads)
{
	omp_set_num_threads(threads);
	clock_t start, stop;
	start = clock();

	vector<int> primes = SieveRange(2, floor(sqrt(arr[arr.size() - 1])) + 1); // Wyznaczenie liczb pierwszych do pierwiastka z wartości maksymalnej
	int primesSize = primes.size();

#pragma omp parallel
	{
#pragma omp for schedule(dynamic, 500) nowait
		for (int i = 0; i < arr.size(); i++)
		{
			int number = arr[i]; // Przypisanie do 'number' sprawdzanej wartości, aby możliwie mało razy odwoływać się bezpośrednio do vectora, co zmniejszy szansę na unieważnienie linii pamięci podręcznej.

			if (number % 2 == 0 && number != 2) // Pomijanie w przetwarzaniu liczb parzystych
			{
				arr[i] = -1;
				continue;
			}

			int sq = floor(sqrt(number)) + 1; // Przypisanie do zmiennej wartości pierwiastka ze sprawdzanej liczby w celu zminimalizowania powtarzających się obliczeń w pętli. Pozwoliło to na przyspieszenie algorytmu o 8-12%.
			bool divided = false;
			for (int j = 0; j < primesSize && !divided; j++) // W trakcie wykonywania pętli, za każdym razem sprawdzane jest, czy dana liczba została już wcześniej podzielona. Jeśli tak - program wychodzi z pętli.
			{
				int t = primes[j];
				if (number % t == 0 && number != t) // Sprawdzanie, czy dana liczba 'number' jest podzielna przez liczbę pierwszą z vectora.
					divided = true;
			}
			if (divided)
				arr[i] = -1; // Oznaczenie, że dana liczba nie jest liczbą pierwszą.
		}
		cout << omp_get_thread_num() << " ";
	}

	stop = clock();

	return (stop - start) / 1000.;
}

double SeqSieve(vector<int>& arr)
{
	clock_t start, stop;
	start = clock();

	int arrSize = arr.size();
	int minimum = arr[0]; // Wyznaczenie minimum vectora i przypisanie do zmiennej w celu rzadszego odwoływania się bezpośrednio do vectora.
	int maximum = arr[arrSize - 1]; // Wyznaczenie maximum vectora i przypisanie do zmiennej w celu rzadszego odwoływania się bezpośrednio do vectora.

	vector<int> primes = SieveRange(2, floor(sqrt(maximum)) + 1); // Wyznaczenie liczb pierwszych z przedziału (2, pierwiastek z maximum przedziału).
	int primesSize = primes.size(); // Wyznaczenie liczby liczb pierwszych do pierwiastka z maximum, w celu uniknięcia nadmiarowych obliczeń w pętli.

	for (int i = 0; i <= maximum - minimum; i += BLOCK_SIZE) // Przetwarzanie blokowe, co BLOCK_SIZE, w celu trzymania w pamięci cache optymalnej ilości danych.
	{
		for (int j = 0; j < primesSize; j++) // Pętla po wszystkich liczbach pierwszych do pierwiastka z maximum.
		{
			int beg = minimum + i; // Wyznaczenie liczby równej początku przetwarzania w danym bloku.
			int t = beg / primes[j] + (beg % primes[j] == 0 ? 0 : 1); // Wyznaczenie od której wielokrotności liczby pierwszej zacząć. Jeśli początek jest od razu wielokrotnością, zaczyna się od niego, w przeciwnym wypadku należy wyznaczyć kolejną wielokrotność, aby była większa niż minimum.
			if (t < 2)
				t = 2; // Gdyby wielokrotność wyszła mniejsza niż 2, zapewnienie, aby była ona przynajmniej równa 2.

			int end = minimum + i + BLOCK_SIZE; // Wyznaczenie liczby równej końca przetwarzania w danym bloku.
			if (end > maximum)
				end = maximum; // Zabezpieczenie przed sytuacją, gdyby przetwarzanie zaszło powyżej maximum przedziału.

			while (t * primes[j] <= end)
			{
				arr[t * primes[j] - minimum] = -1; // Zaznaczanie, że dana liczba nie jest liczbą pierwszą.
				t++;
			}
		}
	}

	stop = clock();

	return (stop - start) / 1000.;
}

double DomainSieve(vector<int>& arr, int threads)
{
	omp_set_num_threads(threads);
	clock_t start, stop;
	start = clock();

	int arrSize = arr.size();
	int min = arr[0];
	int max = arr[arrSize - 1];

	vector<int> primes = SieveRange(2, floor(sqrt(max)) + 1); // Wyznaczenie liczb pierwszych z przedziału (2, pierwiastek z maximum przedziału).
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
		cout << omp_get_thread_num() << " "; // Sprawdzanie zrównoważenia pracy wątków
	}

	stop = clock();

	return (stop - start) / 1000.;
}

double FunctionSieve(vector<int>& arr, int threads)
{
	omp_set_num_threads(threads);
	clock_t start, stop;
	start = clock();

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
		cout << omp_get_thread_num() << " "; // Sprawdzanie zrównoważenia pracy wątków
	}

	stop = clock();

	return (stop - start) / 1000.;
}

int main()
{

	if (SINGLE1_OR_ALL2 == 1)
	{
		vector<int> vec;
		SetUpArray(vec, SINGLE_CHOOSE_RANGE123);

		string rangeNumString;
		if (SINGLE_CHOOSE_RANGE123 == 1)
			rangeNumString = "pierwszego";
		else if(SINGLE_CHOOSE_RANGE123 == 2)
			rangeNumString = "drugiego";
		else if (SINGLE_CHOOSE_RANGE123 == 3)
			rangeNumString = "trzeciego";

		if (FIRST1_OR_SIEVE2 == 1)
		{
			//cout << "Pierwszy algorytm sekwencyjnie dla " << rangeNumString << " przedzialu: " << SeqPrime(vec) << "\n";
			cout << "Pierwszy algorytm sekwencyjnie dla " << rangeNumString << " przedzialu: " << SeqPrimeImproved(vec) << "\n";
			ShowResults(vec);

			SetUpArray(vec, SINGLE_CHOOSE_RANGE123);
			//cout << "Pierwszy algorytm rownolegle (3 procesory) dla " << rangeNumString << " przedzialu: " << ParPrime(vec, 3) << "\n";
			cout << "Pierwszy algorytm rownolegle (3 procesory) dla " << rangeNumString << " przedzialu: " << ParPrimeImproved(vec, 3) << "\n";
			ShowResults(vec);

			SetUpArray(vec, SINGLE_CHOOSE_RANGE123);
			//cout << "Pierwszy algorytm rownolegle (6 procesorow) dla " << rangeNumString << " przedzialu: " << ParPrime(vec, 6) << "\n";
			cout << "Pierwszy algorytm rownolegle (6 procesorow) dla " << rangeNumString << " przedzialu: " << ParPrimeImproved(vec, 6) << "\n";
			ShowResults(vec);
		}
		else if (FIRST1_OR_SIEVE2 == 2)
		{
			cout << "Sito Eratostenesa sekwencyjnie dla " << rangeNumString << " przedzialu: " << SeqSieve(vec) << "\n";
			ShowResults(vec);

			SetUpArray(vec, SINGLE_CHOOSE_RANGE123);

			if (DOMAIN1_OR_FUNCTION2 == 1)
			{
				cout << "Sito Eratostenesa rownolegle (3 procesory) podejsciem domenowym dla " << rangeNumString << " przedzialu: " << DomainSieve(vec, 3) << "\n";
				ShowResults(vec);

				SetUpArray(vec, SINGLE_CHOOSE_RANGE123);
				cout << "Sito Eratostenesa rownolegle (6 procesorow) podejsciem domenowym dla " << rangeNumString << " przedzialu: " << DomainSieve(vec, 6) << "\n";
				ShowResults(vec);
			}
			else if (DOMAIN1_OR_FUNCTION2 == 2)
			{
				cout << "Sito Eratostenesa rownolegle (3 procesory) podejsciem funkcyjnym dla " << rangeNumString << " przedzialu: " << FunctionSieve(vec, 3) << "\n";
				ShowResults(vec);

				SetUpArray(vec, SINGLE_CHOOSE_RANGE123);
				cout << "Sito Eratostenesa rownolegle (6 procesorow) podejsciem funkcyjnym dla " << rangeNumString << " przedzialu: " << FunctionSieve(vec, 6) << "\n";
				ShowResults(vec);
			}
		}
	}
	else if (SINGLE1_OR_ALL2 == 2)
	{
		vector<int> first;
		vector<int> second;
		vector<int> third;

		SetUpArray(first, 1);
		SetUpArray(second, 2);
		SetUpArray(third, 3);

		int firstMin = (first)[0], firstMax = (first)[first.size() - 1];
		int secondMin = (second)[0], secondMax = (second)[second.size() - 1];
		int thirdMin = (third)[0], thirdMax = (third)[third.size() - 1];

		if (FIRST1_OR_SIEVE2 == 1)
		{
			cout << "Pierwszy algorytm sekwencyjnie dla pierwszego przedzialu: " << SeqPrime(first) << "\n";
			cout << "Pierwszy algorytm sekwencyjnie dla drugiego przedzialu: " << SeqPrime(second) << "\n";
			cout << "Pierwszy algorytm sekwencyjnie dla trzeciego przedzialu: " << SeqPrime(third) << "\n\n";

			ShowResults(first, firstMin, firstMax, true);
			ShowResults(second, secondMin, secondMax, true);
			ShowResults(third, thirdMin, thirdMax, true);


			SetUpArray(first, 1);
			SetUpArray(second, 2);
			SetUpArray(third, 3);

			cout << "Pierwszy algorytm rownolegle (3 procesory) dla pierwszego przedzialu: " << ParPrime(first, 3) << "\n";
			cout << "Pierwszy algorytm rownolegle (3 procesory) dla drugiego przedzialu: " << ParPrime(second, 3) << "\n";
			cout << "Pierwszy algorytm rownolegle (3 procesory) dla trzeciego przedzialu: " << ParPrime(third, 3) << "\n\n";

			ShowResults(first, firstMin, firstMax, true);
			ShowResults(second, secondMin, secondMax, true);
			ShowResults(third, thirdMin, thirdMax, true);


			SetUpArray(first, 1);
			SetUpArray(second, 2);
			SetUpArray(third, 3);

			cout << "Pierwszy algorytm rownolegle (6 procesorow) dla pierwszego przedzialu: " << ParPrime(first, 6) << "\n";
			cout << "Pierwszy algorytm rownolegle (6 procesorow) dla drugiego przedzialu: " << ParPrime(second, 6) << "\n";
			cout << "Pierwszy algorytm rownolegle (6 procesorow) dla trzeciego przedzialu: " << ParPrime(third, 6) << "\n\n";

			ShowResults(first, firstMin, firstMax);
			ShowResults(second, secondMin, secondMax);
			ShowResults(third, thirdMin, thirdMax);
		}
		else if (FIRST1_OR_SIEVE2 == 2)
		{
			SetUpArray(first, 1);
			SetUpArray(second, 2);
			SetUpArray(third, 3);

			cout << "Sito Eratostenesa sekwencyjnie dla pierwszego przedzialu: " << SeqSieve(first) << "\n";
			cout << "Sito Eratostenesa sekwencyjnie dla drugiego przedzialu: " << SeqSieve(second) << "\n";
			cout << "Sito Eratostenesa sekwencyjnie dla trzeciego przedzialu: " << SeqSieve(third) << "\n\n";

			ShowResults(first, firstMin, firstMax);
			ShowResults(second, secondMin, secondMax);
			ShowResults(third, thirdMin, thirdMax);

			if (DOMAIN1_OR_FUNCTION2 == 1)
			{
				SetUpArray(first, 1);
				SetUpArray(second, 2);
				SetUpArray(third, 3);

				cout << "Sito Eratostenesa rownolegle (3 procesory) podejsciem domenowym dla pierwszego przedzialu: " << DomainSieve(first, 3) << "\n";
				cout << "Sito Eratostenesa rownolegle (3 procesory) podejsciem domenowym dla drugiego przedzialu: " << DomainSieve(second, 3) << "\n";
				cout << "Sito Eratostenesa rownolegle (3 procesory) podejsciem domenowym dla trzeciego przedzialu: " << DomainSieve(third, 3) << "\n\n";

				ShowResults(first, firstMin, firstMax);
				ShowResults(second, secondMin, secondMax);
				ShowResults(third, thirdMin, thirdMax, false);


				SetUpArray(first, 1);
				SetUpArray(second, 2);
				SetUpArray(third, 3);

				cout << "Sito Eratostenesa rownolegle (6 procesory) podejsciem domenowym dla pierwszego przedzialu: " << DomainSieve(first, 6) << "\n";
				cout << "Sito Eratostenesa rownolegle (6 procesory) podejsciem domenowym dla drugiego przedzialu: " << DomainSieve(second, 6) << "\n";
				cout << "Sito Eratostenesa rownolegle (6 procesory) podejsciem domenowym dla trzeciego przedzialu: " << DomainSieve(third, 6) << "\n\n";

				ShowResults(first, firstMin, firstMax);
				ShowResults(second, secondMin, secondMax);
				ShowResults(third, thirdMin, thirdMax);
			}
			else if (DOMAIN1_OR_FUNCTION2 == 2)
			{
				SetUpArray(first, 1);
				SetUpArray(second, 2);
				SetUpArray(third, 3);

				cout << "Sito Eratostenesa rownolegle (3 procesory) podejsciem funkcyjnym dla pierwszego przedzialu: " << FunctionSieve(first, 3) << "\n";
				cout << "Sito Eratostenesa rownolegle (3 procesory) podejsciem funkcyjnym dla drugiego przedzialu: " << FunctionSieve(second, 3) << "\n";
				cout << "Sito Eratostenesa rownolegle (3 procesory) podejsciem funkcyjnym dla trzeciego przedzialu: " << FunctionSieve(third, 3) << "\n\n";

				ShowResults(first, firstMin, firstMax);
				ShowResults(second, secondMin, secondMax);
				ShowResults(third, thirdMin, thirdMax);


				SetUpArray(first, 1);
				SetUpArray(second, 2);
				SetUpArray(third, 3);

				cout << "Sito Eratostenesa rownolegle (6 procesory) podejsciem funkcyjnym dla pierwszego przedzialu: " << FunctionSieve(first, 6) << "\n";
				cout << "Sito Eratostenesa rownolegle (6 procesory) podejsciem funkcyjnym dla drugiego przedzialu: " << FunctionSieve(second, 6) << "\n";
				cout << "Sito Eratostenesa rownolegle (6 procesory) podejsciem funkcyjnym dla trzeciego przedzialu: " << FunctionSieve(third, 6) << "\n\n";

				ShowResults(first, firstMin, firstMax);
				ShowResults(second, secondMin, secondMax);
				ShowResults(third, thirdMin, thirdMax);
			}
		}
	}
}