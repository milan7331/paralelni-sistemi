#include "april2023.h"

int april2023zadatak4(int argc, char* argv[])
{
    // Implementirati OpenMP C++ rešenje za paralelno množenje dve matrice.

// Zadatak je jako sličan primeru sa računskih vežbi gde množimo matricu i vektor.

    int M;                              // dimenzija matrice
    int N;                              // dimenzija matrice
    int K;                              // dimenzija matrice

    int* a;                             // početna matrica A dimenzija M x N
    int* b;                             // početna matrica B dimenzija N x K
    int* c;                             // rezultujuća matrica C dimenzija M x K

    // Unos dimenzija matrica
    std::cout << "Množenje matrice A dimenzija M x N matricom B dimenzija N x K:" << std::endl;
    std::cout << "Uneti vrednost dimenzije M: ";
    std::cin >> M;
    std::cout << "Uneti vrednost dimenzije N: ";
    std::cin >> N;
    std::cout << "Uneti vrednost dimenzije K: ";
    std::cin >> K;

    // Alokacija memorije za matrice odabranih dimenzija
    a = new int[M * N];
    b = new int[N * K];
    c = new int[M * K];

    // Popuna matrica inicijalnim vrednostima. Matrica B je popunjena inicijalnim vrednostima tako da lako možemo
    // da vidimo iz rezultata da li je množenje uspešno izvršeno.
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            a[i * N + j] = i * N + j;
        }
    }

    for (int i = 0; i < N * K; i++)
    {
        b[i] = 2;
    }

    for (int i = 0; i < M * K; i++)
    {
        c[i] = 0;
    }

    // paralelizacija izračunavanja po jednoj dimenziji
    // kada promenljive i,j i l deklarišemo u samim petljama nema potrebe za private(j, l) odredbom
#pragma omp parallel for num_threads(M)
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < K; j++)
        {
            for (int l = 0; l < N; l++)
            {
                c[i * K + j] += a[i * N + l] * b[l * K + j];
            }
        }
    }

    // Matrice A, B i C se ispisuju na ekran radi provere tačnosti
    std::cout << "Matrica A: " << std::endl;
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            std::cout << a[i * N + j] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Matrica B: " << std::endl;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < K; j++)
        {
            std::cout << b[i * K + j] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Matrica C: " << std::endl;
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < K; j++)
        {
            std::cout << c[i * K + j] << " ";
        }
        std::cout << std::endl;
    }

    // kraj zadatka, oslobađamo memoriju
    delete[] a;
    delete[] b;
    delete[] c;

    return 0;
}