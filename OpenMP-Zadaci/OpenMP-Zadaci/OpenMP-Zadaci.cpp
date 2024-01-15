#include "OpenMP-Zadaci.h"

int main(int argc, char* argv[])
{
    // ISPITNI ZADACI IZ 2023


    // ZADACI SA PREZENTACIJA
    // zadatak1();
    // zadatak2();
    // zadatak3();
    // zadatak4();
    // zadatak5();
    // zadatak6();
    // zadatak7();
    // zadatak8();
    // zadatak9();
    zadatak10();


    return 0;
}

int zadatak1()
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

int zadatak2()
{
    omp_set_num_threads(15);

    #pragma omp parallel
    {
        std::cout << "Hello from thread " << omp_get_thread_num() << std::endl;

        #pragma omp single
        {
            std::cout << "There are " << omp_get_num_threads() << " threads in total." << std::endl;
        }
    }

    return 0;
}

int zadatak3()
{
    omp_set_num_threads(15);

    #pragma omp parallel
    {
        #pragma omp critical
        {
            std::cout << "Hello from thread " << omp_get_thread_num() << std::endl;
        }

        #pragma omp single
        {
            #pragma omp critical
            {
                std::cout << "There are " << omp_get_num_threads() << " threads in total." << std::endl;
            }
            
        }
    }
    return 0;
}

int zadatak4()
{
    int factorial = 1;
    int number;
    std::cout << "Uneti broj ciji faktorijel racunamo: ";
    std::cin >> number;

    #pragma omp parallel for
    for (int n = 2; n <= number; n++)
    {
        #pragma omp critical
        factorial *= n;
    }

    std::cout << "Faktorijel je: " << factorial << std::endl;

    return 0;
}

int zadatak5()
{
    int factorial = 1;
    int number;
    std::cout << "Uneti broj ciji faktorijel racunamo: ";
    std::cin >> number;

    #pragma omp parallel for reduction(*:factorial)
    for (int n = 2; n <= number; n++)
    {
        factorial *= n;
    }

    std::cout << "Faktorijel je: " << factorial << std::endl;

    return 0;
}

int zadatak6()
{
    // množenje matrice b vektorom c -> zapisuje se u a

    double* a;
    double* b;
    double* c;
    int i;
    int j;
    int m;
    int n;

    std::cout << "Uneti m: ";
    std::cin >> m;
    std::cout << "Uneti n: ";
    std::cin >> n;

    a = new double[m];
    b = new double[m * n];
    c = new double[n];

    for (i = 0; i < n; i++)
    {
        c[i] = 2.0;
    }

    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
        {
            b[i * n + j] = i;
        }
    }

    #pragma omp parallel for private(j)
    for (i = 0; i < m; i++)
    {
        a[i] = 0.0;
        for (j = 0; j < n; j++)
        {
            a[i] += b[i * n + j] * c[j];
        }
    }

    for (i = 0; i < m; i++)
    {
        std::cout << a[i] << std::endl;
    }

    delete[] a;
    delete[] b;
    delete[] c;

    return 0;
}

int zadatak7()
{
    // sekvencijalni algoritam - numerička integracija

    static long num_steps = 100000000;
    double step;

    int i;
    double x;
    double pi;
    double sum = 0.0;

    step = 1.0 / (double)num_steps;

    for (i = 1; i < num_steps; i++)
    {
        x = (i - 0.5) * step;
        sum = sum + 4.0 / (1.0 + x * x);
    }
    pi = step * sum;
    
    std::cout << "Pi sekvencijalno: " << pi << std::endl;

    return 0;
}

int zadatak8()
{
    // paralelni algoritam 1 - numerička integracija

    const int NUM_THREADS = 10;

    static long num_steps = 100000000;
    double step;

    int i = 0;
    double x;
    double pi;
    double sum = 0.0;

    step = 1.0 / (double)num_steps;
    omp_set_num_threads(NUM_THREADS);

    #pragma omp parallel private(x)
    {
        #pragma omp for
        for (i = 1; i <= num_steps; i++)
        {
            x = (i - 0.5) * step;
            #pragma omp critical
            {
                sum += 4.0 / (1.0 + x * x);
            }
        }
    }

    pi = sum * step;
    std::cout << "Pi: " << pi << std::endl;

    return 0;
}

int zadatak9()
{
    // paralelni algoritam 2 - numerička integracija
    // radi mnogo bolje nego prethodni
    // kritične sekcije nisu poželjne unutar petlji
    // jer ispada da se onda izvršavaju sekvencijalno

    const int NUM_THREADS = 10;

    static long num_steps = 100000000;
    double step;

    int i = 0;
    double x;
    double pi = 0.0;
    double sum;

    step = 1.0 / (double)num_steps;
    omp_set_num_threads(NUM_THREADS);

    #pragma omp parallel private(x, sum)
    {
        sum = 0.0;
        #pragma omp for
        for (i = 1; i <= num_steps; i++)
        {
            x = (i - 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }   
        #pragma omp critical
        {
            pi += sum * step;
        }
    }

    std::cout << "Pi: " << pi << std::endl;

    return 0;
}

int zadatak10()
{
    // paralelni algoritam 3 - numerička integracija
    // reduction pristup
    const int NUM_THREADS = 2;

    static long num_steps = 100000000;
    double step;

    int i;
    double x;
    double pi;
    double sum = 0.0;

    step = 1.0 / (double)num_steps;
    omp_set_num_threads(NUM_THREADS);

    #pragma omp parallel for reduction(+:sum) private(x)
    for (i = 1; i <= num_steps; i++)
    {
        x = (i - 0.5) * step;
        sum = sum + 4.0 / (1.0 + x * x);
    }
    pi = step * sum;

    std::cout << "Pi: " << pi << std::endl;

    return 0;
}