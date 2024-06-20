#include "oktobar22023.h"

int oktobar22023zadatak3()
{
    // a) Koliko se taskova generiše u navedenom OpenMP primeru i da li je redosled izvršenja taskova definisan ? Obrazložiti.
    //
    // omp_set_num_threads(3);
    // #pragma omp parallel
    // {
    //     #pragma omp task
    //     {
    //         printf("Task 1\n");
    //     }
    //     #pragma omp task
    //     {
    //         printf("Task 2\n");
    //     }
    // }
    //
    // b) Proučiti da li je moguće izvršiti paralelizaciju sledeće petlje:
    // 
    // for (i = N - 1; i > 1; i--)
    // {
    //     a += okt[i] + okt2[i];
    //     rok[i] = rok[i - 1] + a;
    // }
    //
    //Ako je paralelizacija moguća (sa ili bez transformacije petlje), napisati OpenMP kod koji sadrži petlju. U suprotnom
    //obrazložiti zašto paralelizacija nije moguća. Razvijanjem petlje sekvencijalnog i paralelnog rešenja i prolaskom
    //kroz iteracije za proizvoljno N i proizvoljan broj niti, pokazati korektnost paralelizovanog koda.

    return 0;
}