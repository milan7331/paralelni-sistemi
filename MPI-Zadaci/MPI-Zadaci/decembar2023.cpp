#include "decembar2023.h"

struct double_int
{
    double value;
    int rank;
};

int decembar2023zadatak1(int argc, char* argv[])
{
    // Napisati MPI program koji ralizuje množenje matrice A kxm i vektora Bm. matrica A se inicijalizuje u master
    // procesu i podeljena je u blokove po kolonama (m je deljivo sa p, pri čemu je p broj procesa), tako da proces i
    // dobija kolone sa l, l mod p = i (0 <= i <= p-1), tj. kolone sa indeksima i, i + p, i + 2p, ... , i + n - p
    // Vektor B treba učitati iz fajla "B-total.dat" tako da proces Pi učita elemente s indeksima l, l mod p = i (0 <= i <= p-1),
    // tj. elemente sa indeksima i, i + p, i + 2p, ... , i + n - p???. Voditi računa o efikasnosti učitavanja iz fajla.
    // Predvideti da se slanje blokova matrica svakom procesu, kao i čitanje elemenata vektora obavlja odjednom.
    // Svaki proces obavlja odgovarajuća izračunavanja i učestvuje u generisanju rezultata. Rezultujući vektor se treba
    // naći u procesu koji je učitao maksimalni element vektora B. Zadatak rešiti korišćenjem grupnih operacija,
    // izvedenih tipova podataka i funkcija za rad sa fajlovima.
    // Dati primer matrice A i vektora B i ilustrovati podelu podataka po procesima. Napisati na koji bi se način iz
    // komandne linije vršilo startovanje napisane MPI aplikacije za izabrane dimenzije iz primera.

    // Napomena: zadatak pokrenuti sa brojem procesa p = 8
    // Napomena: potrebno je prvi put pokrenuti prvo helper metodu za kreiranje fajla koji sadrži vektor B

    constexpr int k = 10;               // Broj redova matrice A
    constexpr int m = 24;               // Broj kolona matrice A

    int A[k][m]{};                      // Glavna matrica A koja se dalje deli procesima za izračunavanje
    int B[m]{};                         // Vektor B koji se učitava iz fajla i deli procesima za izračunavanje
    int C[k]{};                         // Finalni rezultat množenja

    int* localA;                        // Lokalna matrica za smeštanje dela glavne matrice A, potrebna za računicu
    int* localB;                        // Lokalni vektor za smeštanje dela vektora B, potreban za računicu
    int* localC;                        // Međurezultat množenja određenih kolona matrice A i dela vektora B

    int p;                              // Broj procesa P
    int rank;                           // Id procesa u glavnom komunikatoru
    
    int colNum;                         // Broj kolona matrice A koje dobija svaki proces za izračunavanje

    MPI_Datatype sendMatType;           // Izvedeni tip za slanje delova matrice A odgovarajućim procesima
    MPI_Datatype sendMatTypeResized;    // Izvedeni tip za pravilno slanje delova matrice A odgovarajućim procesima
    MPI_Datatype readFileType;          // Izvedeni tip za učitavanje delova vektora B iz fajla

    MPI_File fh;                        // File handle za dokument iz kog čitamo vektor B

    double_int localMax;                // Struktura koja sadrži maksimalni element vektora B i ID procesa
    double_int globalMax;               // Struktura koja sadrži globalni maksimalni element vektora B i ID procesa
                                        // koji ga sadrži

    // MPI Init
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    // Određujemo broj kolona matrice A + dužinu vektora B koje učiva svaki proces i alociramo potrebnu memoriju.
    colNum = m / p;
    localA = new int[k * colNum] {};
    localB = new int[colNum] {};
    localC = new int[k] {};

    // Kreiramo potrebne izvedene tipove za slanje delova matrice A procesima
    MPI_Type_vector(colNum * k, 1, p, MPI_INT, &sendMatType);
    MPI_Type_commit(&sendMatType);
    MPI_Type_create_resized(sendMatType, 0, 1 * sizeof(int), &sendMatTypeResized);
    MPI_Type_commit(&sendMatTypeResized);

    // Proces 0 inicijalizuje vrednosti matrice
    if (rank == 0)
    {
        for (auto i = 0; i < k; i++)
        {
            for (auto j = 0; j < m; j++)
            {
                A[i][j] = i * m + j;
            }
        }
    }

    // Šaljemo ovu matricu ostalim procesima
    MPI_Scatter(A, 1, sendMatTypeResized, localA, k * colNum, MPI_INT, 0, MPI_COMM_WORLD);

    // Kreiramo potrebne izvedene tipove za učitavanje vrednosti iz fajla
    MPI_Type_vector(colNum, 1, p, MPI_INT, &readFileType);
    MPI_Type_commit(&readFileType);

    // Procesi optimalno učitavaju odgovarajuće vrednosti vektora B iz zadatog fajla
    MPI_File_open(MPI_COMM_WORLD, "B-total.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_File_set_view(fh, rank * sizeof(int), MPI_INT, readFileType, "native", MPI_INFO_NULL);
    MPI_File_read_all(fh, localB, colNum, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    // Procesi izračunavaju svoje lokalne rezultate
    for (auto i = 0; i < k; i++)
    {
        for (auto j = 0; j < colNum; j++)
        {
            localC[i] += (localA[i * colNum + j] * localB[j]);
        }
    }

    // Potrebno je odrediti koji proces je učitao maksimalni element vektora B
    // Maksimalni element će se naći u procesu 0 koji dalje prosleđuje ovu vrednost svim procesima
    localMax.rank = rank;
    localMax.value = std::numeric_limits<int>::min();
    globalMax.rank = 0;
    globalMax.value = std::numeric_limits<int>::min();
    for (auto i = 0; i < colNum; i++)
    {
        if (localB[i] > localMax.value) localMax.value = localB[i];
    }
    MPI_Reduce(&localMax, &globalMax, 1, MPI_DOUBLE_INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);
    MPI_Bcast(&globalMax, 1, MPI_DOUBLE_INT, 0, MPI_COMM_WORLD);

    // Nakon što utvrdimo koji proces ima maksimalnu vrednost vektora B, možemo da izvršimo finalnu redukciju, tj.
    // da prikupimo rezultate množenja
    MPI_Reduce(localC, C, k, MPI_INT, MPI_SUM, globalMax.rank, MPI_COMM_WORLD);

    if (rank == globalMax.rank)
    {
        std::cout << "Rezultat množenja matrice A i vektora B je:" << std::endl;
        for (auto i = 0; i < k; i++)
        {
            std::cout << C[i] << " ";
        }
        std::cout << std::endl;
    }
    
    // Kraj zadatka, oslobađamo memoriju
    delete[] localA;
    delete[] localB;
    delete[] localC;
    MPI_Type_free(&sendMatType);
    MPI_Type_free(&sendMatTypeResized);
    MPI_Type_free(&readFileType);
    MPI_Finalize();

    return 0;
}

// Helper metoda za upis u fajl / generisanje fajla potrebnog za zadatak.
int mpiWriteDec2023(int argc, char* argv[])
{
    constexpr int m = 24;
    int B[m]{};
    
    int rank;
    MPI_File fh;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        for (auto i = 0; i < m; i++)
        {
            B[i] = 9 + 2 * i;
        }

        MPI_File_open(MPI_COMM_WORLD, "B-total.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
        MPI_File_write(fh, B, m, MPI_INT, MPI_STATUS_IGNORE);
        MPI_File_close(&fh);
    }
    
    MPI_Finalize();

    return 0;
}