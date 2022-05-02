#include <cstdio>
#include <ctime>
#include <iostream>
#include <omp.h>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cmath>

using namespace std;

#define MAX 100000
#define MIN 2 

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

void set_up_vector(vector<int> &arr, int min, int max)
{
    for(int i = min; i <= max; ++i)
        arr.emplace_back(i);
}

double seqential_primes(vector<int> &arr)
{
    for(int i = 0; i < arr.size(); ++i)
    {
        // Przypisanie do 'number' sprawdzanej wartości, 
        // aby możliwie mało razy odwoływać się bezpośrednio do vectora 
        // (przydatne przy algorytmie równoległym).
        int number = arr[i]; 

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
}

vector<int> sieve_in_range(int min, int max)
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

void sequential_prime_improved(vector<int>& arr)
{
	vector<int> primes = sieve_in_range(2, floor(sqrt(arr[arr.size() - 1])) + 1); // Wyznaczenie liczb pierwszych do pierwiastka z wartości maksymalnej
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
}

int main()
{
    std::vector<int> array;
    set_up_vector(array, MIN, MAX);

  	seqential_primes(array);

    // show_results(array);

    return 0;
}