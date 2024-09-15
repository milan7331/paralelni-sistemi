#include "jun2022.h"

int jun2022zadatak2(int argc, char* argv[]) {
    // Napisati MPI program koji realizuje množenje matrice A dimenzija n x n i matrice B dimenzija n x n, čime se
    // dobija rezultujuća matrica C dimenzija n x n. Matrice A i B se inicijalizuju u master procesu. Broj procesa je p
    // i uređeni su kao matrica dimenzija q x q (q^2 = p). Na početku matrice A i B su podeljene u blokove dimenzija
    // k x k (k = n / q) i master proces distribuira odgovarajuće blokove matrica A i B po procesima kao što je
    // prikazano na slici 1. za n = 8 i p = 16. Slanje elemenata svakog bloka (k x k elemenata) se obavlja odjednom. Za
    // ovu inicijalnu distribuciju podataka dozvoljeno je koristiti Point-to-point operacije. Nakon inicijalne distri-
    // bucije podataka, korišćenjem grupnih operacija treba obezbediti da svaki proces sadrži sve blokove podataka
    // matrice A iz procesa koji se nalaze u istoj vrsti. Dakle, treba obezbediti da svi procesi u prvoj vrsti matrice
    // procesa sadrže prvih n / q vrsta matrice A, svi procesi u drugoj vrsti matrice procesa sledećih n / q vrsta
    // matrice A itd. Takođe, nakon inicijalne distribucije podataka, korišćenjem grupnih operacija treba obezbediti da
    // svaki proces sadrži sve blovoe podataka matrice B iz procesa koji se nalaze u istoj koloni procesa. Dakle, treba
    // obezbediti da procesi u prvoj koloni matrice procesa sadrže prvih n / q kolona matrice B, procesi u drugoj
    // koloni matrice procesa sledećih n / q kolona matrice B. Sada, na osnovu podataka koje sadrži, svaki proces
    // obavlja odgovarajuća izračunavanja i učestvuje u generisanju rezultata koji se prikazuje u master procesu. Za
    // slanje blokova matrice A i B (osim inicijalnog slanja) i generisanje rezultata koristiti isključivo grupne
    // operacije i funkcije za kreiranje novih komunikatora.

    constexpr int n = 8;                // Glavna dimenzija matrica, sve su kvadratne

    int A[n][n]{};                      // Matrica A - koristi se za slanje
    int B[n][n]{};                      // Matrica B - koristi se za slanje
    int C[n][n]{};                      // Matrica C - glavni rezultat množenja matrica

    int rank;                           // rank procesa u glavnom komunikatoru MPI_COMM_WORLD
    int iRow;                           // lokacija procesa u matrici procesora - broj reda
    int jCol;                           // lokacija procesa u matrici procesora - broj kolone
    int p;                              // ukupan broj procesa koji izvršavaju ovaj program
    int q;                              // broj procesa po redu (koloni)
    int k;                              // dimenzija inicijalnog bloka koji dobijaju svi procesi od master procesa

    int* localA;                        // lokalni blok koji se koristi za formiranje veceg bloka kolona
    int* localB;                        // lokalni blok koji se koristi za formiranje veceg bloka redova
    int* rowsA;                         // veci blok k * n, koristi se pri izracunavanju lokalnih rezultata matrice C
    int* colsB;                         // veci blok n * k, koristi se pri izracunavanju lokalnih rezultata matrice C
    int* localC;                        // lokalni rezultat mnozenja matrica

    MPI_Datatype block;                 // izvedeni tip koji koristimo za formiranje bloka k x k
    MPI_Datatype blockResized;          // !!!
    MPI_Comm rowComm;                   // komunikator koji se koristi za komunikaciju procesa u istom redu
    MPI_Comm colComm;                   // komunikator koji se koristi za komunikaciju procesa u istoj koloni

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    q = static_cast<int>(sqrt(p));
    k = n / q;

    if (rank == 0) {
        for (auto i = 0; i < n * n; i++) {
            A[i / n][i % n] = i % 9;
            B[i / n][i % n] = 2;
        }
    }

    localA = new int[k * k] {};
    localB = new int[k * k] {};
    localC = new int[k * k] {};
    rowsA = new int[k * n] {};
    colsB = new int[n * k] {};

    MPI_Type_vector(k, k, n, MPI_INT, &block);
    MPI_Type_commit(&block);
    MPI_Type_create_resized(block, 0, k * sizeof(int), &blockResized);
    MPI_Type_commit(&blockResized);

    if (rank == 0) {
        for (auto i = 0; i < k; i++) {
            for (auto j = 0; j < k; j++) {
                localA[i * k + j] = A[i][j];
                localB[i * k + j] = B[i][j];
            }
        }

        for (auto i = 0; i < q; i++) {
            for (auto j = 0; j < q; j++) {
                if (i == 0 && j == 0) continue;

                MPI_Send(&A[i * k][j * k], 1, block, i * q + j, 0, MPI_COMM_WORLD);
                MPI_Send(&B[i * k][j * k], 1, block, i * q + j, 1, MPI_COMM_WORLD);
            }
        }
    }
    else {
        MPI_Recv(localA, k * k, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(localB, k * k, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    iRow = rank / q;
    jCol = rank % q;

    MPI_Comm_split(MPI_COMM_WORLD, iRow, jCol, &rowComm);
    MPI_Comm_split(MPI_COMM_WORLD, jCol, iRow, &colComm);

    // !
    MPI_Gather(localA, k * k, MPI_INT, rowsA, 1, blockResized, 0, rowComm);
    MPI_Gather(localB, k * k, MPI_INT, colsB, k * k, MPI_INT, 0,colComm);

    MPI_Bcast(rowsA, k * n, MPI_INT, 0, rowComm);
    MPI_Bcast(colsB, n * k, MPI_INT, 0, colComm);

    for (auto i = 0; i < k; i++) {
        for (auto j = 0; j < k; j++) {
            for (auto x = 0; x < n; x++) {
                localC[i * k + j] += rowsA[i * n + x] * colsB[x * k + j];
            }
        }
    }

    MPI_Gather(localC, k * k, MPI_INT, rowsA, 1, blockResized, 0, rowComm);
    if (jCol == 0) {
        MPI_Gather(rowsA, k * n, MPI_INT, C, k * n, MPI_INT, 0, colComm);
    }

    if (rank == 0) {
        for (auto i = 0; i < n; i++) {
            for (auto j = 0; j < n; j++) {
                std::cout << C[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    delete[] localA;
    delete[] localB;
    delete[] rowsA;
    delete[] colsB;
    delete[] localC;
    MPI_Type_free(&block);
    MPI_Type_free(&blockResized);

    MPI_Finalize();
    return 0;
}