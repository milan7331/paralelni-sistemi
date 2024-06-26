#include "januar2024.h"

int januar2024zadatak1(int argc, char* argv[])
{
    // Napisati MPI program koji realizuje množenje matrice A kxn i vektora bn. Matrica i vektor se inicijalizuju u
    // master procesu. Matrica je podeljena u blokove po vrstama tako da će proces Pi dobiti prvih 2 ^ i vrsta, proces
    // Proces Pi+1 dobiće sledećih 2 ^ i+1 vrsta, itd. Vektor b se u celosti šalje svim procesima. Predvideti da se
    // slanje blokova matrice svakom procesu šalje jednim MPI_Send pozivom kojim se šalju svi neophodni elementi matrice,
    // dok se slanje vektora b obavlja grupnim operacijama. Svaki proces obavlja odgovarajuća izračunavanja i učestvuje
    // u generisanju rezultata. Rezultujući vektor d treba se naći u procesu koji je učitao najviše vrsta matrice A. Dati
    // primer matrice A i vektora b i ilustrovati podelu podataka po procesima, kao i izgled rezultata za izabran broj
    // procesa. Napisati na koji način se iz komandne linije vrši startovanje napisane MPI aplikacije.
    // Napomena: Smatrati da su matrica i vektor dovoljnih dimenzija za uspešno množenje, za izabrani broj procesa.

    // Napomena: zadatak se pokreće sa brojem procesa nProcs = 3
    // Napomena: Potrebno je promeniti i k ako se menja broj procesa. k = (2 ^ n) - 1
    // Ovo ograničenje može da se izbegne ako sve potrebne matrice/nizove alociramo dinamički ali ko ima vreme za to :)

    constexpr int k = 7;                // broj redova matrice
    constexpr int n = 4;                // broj kolona matrice, ujedno i dužina vektora (broj redova)


    int A[k][n]{};                      // glavna matrica A koja se prosleđuje procesima
    int b[n]{};                         // vektor b koji se prosleđuje procesima u celosti
    int d[k]{};                         // rezultujući vektor d

    int pRows;                          // broj redova matrice koje dobija svaki proces za izračunavanje
    int* localA;                        // pointer na lokalnu matricu koju procesi koriste za prihvatanje dela matrice A
                                        // veličina ove matrice se menja u odnosu na id procesa, tj. pRows.
    int* localD;                        // pointer na parcijalni rezultat koji izračunava svaki proces. veličina ovog
                                        // vektora se menja u odnosu na id procesa, tj. pRows

    int rank;                           // identifikator procesa u glavnom komunikatoru MPI_COMM_WORLD
    int nProcs;                         // broj procesa u glavnom comm, ujedno i broj procesa koji izvršava program

    MPI_Datatype sendRowsType;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);

    MPI_Type_contiguous(n, MPI_INT, &sendRowsType);
    MPI_Type_commit(&sendRowsType);

    // Proces 0 inicijalizuje matricu i vektor.
    if (rank == 0)
    {
        for (auto i = 0; i < k; i++)
        {
            for (auto j = 0; j < n; j++)
            {
                A[i][j] = i * n + j;
            }
        }

        for (auto i = 0; i < n; i++)
        {
            b[i] = 1+ i * 2;
        }
    }
    
    // Procesi računaju koliko redova matrice A obrađuju na osnovu njihovog ranka
    // i toliko redova alociraju za lokalnu matricu. Takođe alociraju memoriju za lokalni rezultat.
    pRows = static_cast<int>(pow(2, rank));
    localA = new int[pRows * n] {};
    localD = new int[pRows] {};

    // Proces 0 šalje vrste matrice a, ostali procesi primaju podatke.
    // Prva vrsta se ne šalje već je proces 0 upisuje u svoju lokalnu matricu direktno.
    if (rank == 0)
    {
        for (auto i = 0; i < n; i++)
        {
            localA[i] = A[0][i];
        }

        int pRowsTemp = 0;
        for (auto i = 1; i < nProcs; i++)
        {
            pRowsTemp = static_cast<int>(pow(2, i));
            MPI_Send(&A[pRowsTemp - 1][0], pRowsTemp, sendRowsType, i, 1, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(localA, pRows, sendRowsType, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Vektor b se u celosti šalje svim procesima i koristićemo isti vektor za sve, nema potrebe da alociramo novi
    MPI_Bcast(b, n, MPI_INT, 0, MPI_COMM_WORLD);

    // Procesi računaju deo rezultata za koji su zaduženi
    for (auto i = 0; i < pRows; i++)
    {
        for (auto j = 0; j < n; j++)
        {
            localD[i] += localA[i * n + j] * b[j];
        }
    }

    // Dalje je potrebno odrediti lokaciju (id) rezultujućeg vektora, to možemo da odradimo na više načina:
    // 1. na osnovu vrednosti pRows, koristeći MPI_Reduce sa MPI_MAX operacijom + MPI_Bcast da prosledimo svima max el.
    // 2. možemo da koristimo MPI_Reduce sa MPI_MAXLOC operacijom i odgovarajućom strukturom.
    // 3. intuitivno možemo da zaključimo da će proces sa najvećim indeksom (poslednji u komunikatoru) učitati najviše
    //    redova zato što je pRows = 2 ^ i rastuća funkcija.
    // Biramo 3. način jer je najprostiji.
    // Poslednjem procesu šaljemo odgovarajuće delove rezultujuće matrice.
    MPI_Gather(localD, pRows, MPI_INT, d, n, MPI_INT, nProcs - 1, MPI_COMM_WORLD);

    // Proces koji je sadrži krajnji rezultat ga ispisuje na ekran
    if (rank == nProcs - 1)
    {
        for (auto i = 0; i < n; i++)
        {
            std::cout << d[i] << " ";
        }
        std::cout << std::endl;
    }
    

    // Kraj programa i oslobađanje memorije
    delete[] localA;
    delete[] localD;
    MPI_Type_free(&sendRowsType);
    MPI_Finalize();

    return 0;
}