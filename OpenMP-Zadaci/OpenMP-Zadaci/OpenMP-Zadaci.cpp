#include "OpenMP-Zadaci.h"

int main(int argc, char* argv[])
{
    // ISPITNI ZADACI IZ 2023


    // ZADACI SA PREZENTACIJA
    // zadatak1(argc, argv);

    return 0;
}

int zadatak1(int argc, char* argv[])
{
    int i = 0;
    int a = 0;
    int n = 5;

    omp_set_num_threads(3);

    #pragma omp parallel for private(i) lastprivate(a)
    for (i = 0; i < n; i++)
    {
        a = i + 1;
    }

    std::cout << "Value of 'a' after parallel for: a = " << a << "\n";

    return 0;
}
                                        