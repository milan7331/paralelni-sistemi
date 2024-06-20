#include "januar2023.h"

int januar2023zadatak4(int argc, char* argv[])
{
    // Napisati OpenMP kod koji sadrži sledeće ugnježdene petlje:
    // 
    // x = 0;
    //
    // for (i = 2; i < N; i++)
    // {
    //     for (j = 0; j < M; j++)
    //     {
    //         x = x + a[i][j];
    //         b[i][j] = 4 * b[i-2][j];
    //     }
    // }
    //
    // I proučiti da li je moguće izvršiti njihovu paralelizaciju. Ako nije, transformisati petlje tako da paralelizaci-
    // ja bude moguća. Nakon petlji treba prikazati vrednost za promenljivu x i matricu b, koje su generisane u okviru
    // petlje. Testiranjem sekvencijalnog i paralelnog rešenja za proizvoljno N i M, i proizvoljan broj niti, pokazati
    // korektnost paralelizovanog koda.

    // Napomena: Za "testiranje sekvencijalnog i paralelnog rešenja" je potrebno čisto ručno ispisati par bitnih
    // iteracija petlji, uporediti i iskomentarisati vrednosti kao na slajdovima sa računskih vežbi. (valjda)

    // Zadatak je veoma sličan primerima na kraju prezentacije sa računskih.
    // Operacija sa promenljivom x je nepovezana sa drugom operacijom i moguće je paralelizovati primenom redukcije.
    // Operaciju sa b nizom nije moguće paralelizovati kompletno. Moguće je to uraditi barem po jednoj dimenziji jer
    // postoje zavisnosti po indeksu i. Menjamo indekse obilaska zbog boljih performansi (samo jedan fork-join).

    const int N = 4;                    // broj redova u matricama
    const int M = 3;                    // broj kolona u matricama
    const int thread_num = 10;          // proizvoljan broj niti koje izvršavaju ovaj program

    int x = 0;                          // promenljiva iz primera, koristi se u redukciji

    int i;                              // promenljiva za obilazak petlje
    int j;                              // promenljiva za obilazak petlje

    int a[N][M];                        // matrica iz primera, koristi se u redukciji
    int b[N][M];                        // matrica iz primera, ne može da se paralelizuje obrada

    // inicijalizacija matrica na neke početne vrednosti, bitno samo zbog provere tačnosti programa
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            a[i][j] = i * M + j;
            b[i][j] = i * M + j;
        }
    }

    // paralelno izvršavanje petlje
#pragma omp parallel for reduction(+:x) private(i) shared(a, b)
    for (j = 0; j < M; j++)
    {
        for (i = 2; i < N; i++)
        {
            x = x + a[i][j];
            b[i][j] = 4 * b[i - 2][j];
        }
    }

    // ispisivanje rezultata
    std::cout << "X = " << x << std::endl;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            std::cout << b[i][j] << " ";
        }
        std::cout << std::endl;
    }

    // kraj programa
    return 0;
}