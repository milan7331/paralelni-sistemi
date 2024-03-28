#include "MPI-Zadaci.h"

int main(int argc, char* argv[])
{
    // ISPITNI ZADACI IZ 2023
    // januarZadatak2A(argc, argv);
    // januarZadatak2B(argc, argv);
    // januarZadatak3(argc, argv);
    // aprilZadatak2A(argc, argv);
    // aprilZadatak2B(argc, argv);
    // aprilZadatak3(argc, argv);
    


    //ZADACI SA PREZENTACIJA - NEBITNI & BUĐAVI
    // zadatak1(argc, argv);
    // zadatak2(argc, argv);
    // zadatak3(argc, argv);
    // zadatak4(argc, argv);
    // zadatak5(argc, argv);
    // zadatak6(argc, argv);
    // zadatak8(argc, argv);

    return 0;
}
int januarZadatak2A(int argc, char* argv[])
{
   // Napisati MPI program koji realizuje množenje matrice A dimenzija n x n i matrice B dimenzija n x n, čime se dobija
   // rezultujuća matrica C. Program pronalazi i prikazuje maksimalni element u rezultujućoj matrici, kao i
   // identifikator procesa koji ga sadrži. Matrice A i B se inicijalizuju u master procesu. Matrica A je podeljena na
   // blokove od po k vrsta (k je zadata konstanta i n je deljivo sa k), a matrica B podeljena u blokove od po k kolona.
   // Master proces distribuira odgovarajuće blokove matrica A i odgovarajuće blokove matrice B procesima radnicima.
   // Ukupan broj procesa je (n/k) ^ 2. Svaki proces obavlja odgovarajuća izračunavanja i učestvuje u generisanju
   // rezultata koji se prikazuje u master procesu. Predvideti da se slanje blokova od po k vrsta matrice A i od po k
   // kolona matrice B svakom procesu odvija sa po jednom naredbom MPI_Send kojom se šalje samo jedan izvedeni tip
   // podatka. Za ostatak izračunavanja koristiti grupne operacije.

   // Napomena: predviđeno je da se zadatak pokreće sa nProcs = 25!

    constexpr int N = 10;               // dimenzija kvadratne matrice
    constexpr int K = 2;                // velicina bloka 

    int nProcs;                         // ukupan broj procesa
    int q;                              // broj procesa po jednoj dimenziji matrice

    int rank;                           // id procesa
    int rowRank;                        // id procesa u svom redu
    int colRank;                        // id procesa u svojoj koloni

    int iRow;                           // koordinate procesa u matrici procesa
    int jCol;                           // koordinate procesa u matrici procesa

    int a[N][N] = {};                   // glavna matrica A
    int b[N][N] = {};                   // glavna matrica B
    int c[N][N] = {};                   // retultat mnozenja matrica A i B

    int localA[K][N] = {};              // deo matrice A koji proces dobija za racunicu
    int localB[N][K] = {};              // deo matrice B koji proces dobija za racunicu
    int localC[K][K] = {};              // deo rezultata mnozenja

    MPI_Datatype kRows;                 // blok matrice A koji se salje procesima
    MPI_Datatype kCols;                 // blok matrice B koji se salje procesima
    MPI_Datatype kColsResized;          // blok za vracanje lokalnih rez master procesu

    MPI_Comm rowComm;                   // kanal za komunikaciju procesa u istom redu
    MPI_Comm colComm;                   // kanal za komunikaciju procesa u istoj koloni

    struct MaxValue                     // struktura koju koristimo za pronalazak maksimalne vrednosti
    {                                   
        double value = 0;               // max vrednost
        int rank = 0;                   // lokacija - id procesa koji sadrži ovu vrednost
    };

    MaxValue localMaxValue;             // struktura koja sadrži maksimalni element u procesu
    MaxValue globalMaxValue;            // struktura koja sadrži globalni maksimum

    // mpi inicijalizacija
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);

    // broj procesa po dimenziji
    q = static_cast<int>(sqrt(nProcs));

    // inicijalizacija izvedenih tipova
    MPI_Type_vector(K, N, N, MPI_INT, &kRows);
    MPI_Type_commit(&kRows);
    MPI_Type_vector(N, K, N, MPI_INT, &kCols);
    MPI_Type_commit(&kCols);
    MPI_Type_create_resized(kCols, 0, K * sizeof(int), &kColsResized);
    MPI_Type_commit(&kColsResized);

    // popuna matrice
    if (rank == 0)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                a[i][j] = j + i * N;
                b[i][j] = N * N - i * N - j;
            }
        }
    }

    // slanje odgovarajućih vrsta i kolona odgovarajućim procesima
    // za master proces samo popunjavamo lokalne matrice, ništa ne šaljemo
    if (rank == 0)
    {
        for (int i = 0; i < K; i++)
        {
            for (int j = 0; j < N; j++)
            {
                localA[i][j] = a[i][j];
                localB[j][i] = b[j][i];
            }
        }

        for (int i = 0; i < q; i++)
        {
            for (int j = 0; j < q; j++)
            {
                if (i + j != 0)
                {
                    MPI_Send(&a[i * K][0], 1, kRows, i * q + j, 1, MPI_COMM_WORLD);
                    MPI_Send(&b[0][j * K], 1, kCols, i * q + j, 2, MPI_COMM_WORLD);
                }
            }
        }
    }
    else
    {
        MPI_Recv(&localA, K * N, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&localB, K * N, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // množenje delova matrica za koje je dati proces zadužen
    for (int i = 0; i < K; i++)
    {
        for (int j = 0; j < K; j++)
        {
            for (int x = 0; x < N; x++)
            {
                localC[i][j] += localA[i][x] * localB[x][j];
            }
        }
    }

    // računanje koordinata procesa
    iRow = rank / q;
    jCol = rank % q;

    // inicijalizacija komunikatora za grupne operacije
    MPI_Comm_split(MPI_COMM_WORLD, iRow, jCol, &rowComm);
    MPI_Comm_split(MPI_COMM_WORLD, jCol, iRow, &colComm);
    MPI_Comm_rank(rowComm, &rowRank);
    MPI_Comm_rank(colComm, &colRank);

    // prikupljanje rezultata množenja pomocu grupnih operacija:
    // 1. prikupljamo lokalne rezultate mnozenja u prvi red, koristeći komunikatore kolona
    // 2. sada se u svakom procesu iz prvog reda nalazi blok od k kolona, preko kom reda sakupljamo sve kolone u master
    // proces.
    // Napomena: u prvom koraku za prikupljanje lokalnih rezultata u prvi red ponovo koristimo matricu localB zato što
    // je odgovarajućih dimenzija
    MPI_Gather(&localC, K * K, MPI_INT, &localB, K * K, MPI_INT, 0, colComm);
    if (colRank == 0)
    {
        MPI_Gather(&localB, N * K, MPI_INT, &c, 1, kColsResized, 0, rowComm);
    }

    // Drugi deo zadatka gde pronalazimo maksimalnu vrednost i id procesa
    localMaxValue.rank = rank;
    localMaxValue.value = localC[0][0];

    for (int i = 0; i < K; i++)
    {
        for (int j = 0; j < K; j++)
        {
            if (localMaxValue.value < localC[i][j])
            {
                localMaxValue.value = localC[i][j];
            }
        }
    }

    // vraćamo max vrednost master procesu
    // napomena: postoji i drugi način da se ovo odradi tako što master procesu šaljemo samo max element i onda
    // broadcastujemo taj max element svim procesima -> ako proces sadrži globalni masksimalni element ispisuje rank
    MPI_Reduce(&localMaxValue, &globalMaxValue, 1, MPI_DOUBLE_INT, MPI_MAXLOC , 0, MPI_COMM_WORLD);

    // rezultati
    // ispisujemo rezultujuću matricu na kraju množenja zatim ispisujemo maksimalni element i id procesa koji ga sadrzi
    if (rank == 0)
    {
        std::cout << "Matrica C:" << std::endl;

        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                std::cout << c[i][j] << " ";
            }
            std::cout << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Maksimalni element (" << globalMaxValue.value << ")";
        std::cout << " sadrži proces: " << globalMaxValue.rank << std::endl;
    }

    // kraj zadatka oslobađamo memoriju
    MPI_Type_free(&kRows);
    MPI_Type_free(&kCols);
    MPI_Type_free(&kColsResized);
    MPI_Finalize();

    return 0;
}

int januarZadatak2B(int argc, char* argv[])
{
    // Napisati MPI program kojim se podaci o nizu studenata koji se unose sa tastature u master procesu, prosleđuju
    // odjednom svim procesima u komunikatoru MPI_COMM_WORLD. Za svakog studenta se pamte: broj indeksa (int), ime
    // (string), prezime (string), prosečna ocena (float) i godina studija (int). Zadatak rešiti korišćenjem grupnih
    // operacija i izvedenih tipova podataka.

    // Ovaj zadatak se može pokrenuti sa bilo kojim brojem procesa radnika
    // (ukupan broj procesa >= 2)

    constexpr int N = 2;                // broj studenata koji se unosi sa tastature

    int rank;                           // id procesa u komunikatoru
    int size;                           // ukupan broj procesa u komunikatoru

    struct Student                      // struktura koju korstimo za čuvanje podataka u samom programu
    {
        int indeks = 1000;              // broj indeksa studenta
        std::string ime;                // ime studenta
        std::string prezime;            // prezime studenta
        float prosecnaOcena = 7;        // prosečna ocena studenta
        int godinaStudija = 1;          // trenuta godina studija
    };

    Student studenti[N];                // niz studenata

    MPI_Datatype studentType;           // izvedeni tip koji koristimo za razmenu podataka takođe struktura

    // elementi potrebni za kreiranje studentType izvedenog tipa
    MPI_Datatype types[5];              // niz tipova koje struktura sadrzi
    int blocklens[5];                   // broj elemenata određenog tipa
    MPI_Aint base;
    MPI_Aint displacements[5];          // niz pomeraja svakog bloka

    // popuna potrebnih podataka za kreiranje strukture, nije obavezno ovako eksplicitno to uraditi već možemo i u samom
    // pozivu MPI_Type_struct_create sve, ovo je samo zbog preglednosti
    types[0] = MPI_INT;
    types[1] = MPI_CHAR;
    types[2] = MPI_CHAR;
    types[3] = MPI_FLOAT;
    types[4] = MPI_INT;
    blocklens[0] = 1;
    blocklens[1] = 50;
    blocklens[2] = 50;
    blocklens[3] = 1;
    blocklens[4] = 1;

    // Napomena: ako nije specifično zahtevano offsetof je daleko jednostavniji od MPI_Address
    //displacements[0] = offsetof(Student, indeks);
    //displacements[1] = offsetof(Student, ime);
    //displacements[2] = offsetof(Student, prezime);
    //displacements[3] = offsetof(Student, prosecnaOcena);
    //displacements[4] = offsetof(Student, godinaStudija);

    // mpi inicijalizacija
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2)
    {
        std::cout << "Potrebno je program pokrenuti sa većim brojem procesa!";
        std::cout << std::endl;
        MPI_Finalize();
        return 0;
    }

    // računanje displacementa za naš izvedeni tip korišćenjem MPI_Address, vidi gore za jednostavniju varijantu
    // Napomena: MPI_Address je deprecated i zadatak ne radi s njim već koristimo MPI_Get_Address
    MPI_Get_address(&((Student*)0)->indeks, &base);
    MPI_Get_address(&((Student*)0)->ime, &displacements[1]);
    MPI_Get_address(&((Student*)0)->prezime, &displacements[2]);
    MPI_Get_address(&((Student*)0)->prosecnaOcena, &displacements[3]);
    MPI_Get_address(&((Student*)0)->godinaStudija, &displacements[4]);
    displacements[0] = 0;
    displacements[1] -= base;
    displacements[2] -= base;
    displacements[3] -= base;
    displacements[4] -= base;


    // napokon kreiramo i potvrđujemo našu strukturu
    MPI_Type_create_struct(5, blocklens, displacements, types, &studentType);
    MPI_Type_commit(&studentType);

    // master proces je zadužen za unos studenata i slanje ostalim procesima dok ostali procesi samo primaju podatke
    if (rank == 0)
    {
        for (int i = 0; i < N; i++)
        {
            std::cout << "Uneti detalje za studenta: ";
            std::cout << i + 1 << "/" << N << std::endl;
            std::cout << "Broj indeksa: ";
            std::cin >> studenti[i].indeks;
            std::cout << "Ime: ";
            std::cin >> studenti[i].ime;
            std::cout << "Prezime: ";
            std::cin >> studenti[i].prezime;
            std::cout << "Prosečna ocena: ";
            std::cin >> studenti[i].prosecnaOcena;
            std::cout << "Godina studija: ";
            std::cin >> studenti[i].godinaStudija;
        }
        MPI_Bcast(&studenti, N, studentType, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Bcast(studenti, N, studentType, 0, MPI_COMM_WORLD);
    }

    // radi provere ispravnosti programa uzimamo da jedan proces ispisuje primljene podatke
    if (rank == 1)
    {
        std::cout << "Proces 1 sadrži sledeće podatke: " << std::endl;
        for (int i = 0; i < N; i++)
        {
            std::cout << "Student broj: " << i + 1 << std::endl;
            std::cout << "Indeks: " << studenti[i].indeks << " ";
            std::cout << "Ime: " << studenti[i].ime << " ";
            std::cout << "Prezime: " << studenti[i].prezime << " ";
            std::cout << "Prosečna ocena: " << studenti[i].prosecnaOcena << " ";
            std::cout << "Godina studija: " << studenti[i].godinaStudija << " ";
            std::cout << std::endl;
        }
    }

    // kraj zadatka, oslobađamo memoriju
    MPI_Type_free(&studentType);
    MPI_Finalize();
    return 0;
}

int januarZadatak3(int argc, char* argv[])
{
    // Napomena: zadatak ima sliku!
    // Datoteka file1.dat sadrži ukupno 10MB prikupljenih zapisa o mrežnom saobraćaju. Napisati MPI program koji vrši
    // obradu ovih podataka i priprema ih za vizuelizaciju, ujedno vršeći paralelni upis i čitanje datoteke. Na početku
    // svi procesi vrše čitanje iste količine podataka tako da se pozicija sa koje svaki pojedinačan proces čita podatke
    // ne može predvideti. Pročitane podatke procesi upisuju u datoteku file2.dat tako što ih dele na dva jednaka dela i 
    // upisuju u različite blokove kao na slici (za slučaj od 4 procesa):
    // Obratiti pažnju na efikasnost paralelizacije upisa.

    // Napomena: Potrebno je otkomentarisati deo koda prvi put radi popune fajla file1.dat, a za proveru tačnosti je 
    // potreban neki hex editor recimo HxD

    constexpr int FILESIZE = 10485760;  // veličina fajla koji se čita, u ovom slučaju 10MB = 10 * 1024 * 1024

    int rank;                           // id procesa u komunikatoru
    int size;                           // ukupan broj procesa u komunikatoru

    char* buffer;                       // niz koji koristimo za čitanje i upis podataka
    int bufferSize;                     // veličina niza u bajtovima
    int n;                              // broj elemenata niza (tipa char)

    MPI_Datatype contigiousType;        // izvedeni tip koji koristimo u više navrata - nije potreban
    MPI_Datatype writingViewType;       // izvedeni tip koji služi da kreiramo drugačiji pogled prilikom upisa

    MPI_File file1;                     // handle fajla za čitanje file1.dat
    MPI_File file2;                     // handle fajla za upis file2.dat

    // mpi inicijalizacija
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // izračunavamo veličinu buffera koji procesi koriste za I/O operacije
    // Napomena: postoji drugi način da se odradi inicijalizacija buffera ako fajl nije tačno 10MB
    // MPI_File_get_size(file1, &fileSize); ali to nije rađeno na času tako da ??
    if (FILESIZE % size != 0 && rank == size - 1)
    {
        bufferSize = FILESIZE % size;
    }
    else
    {
        bufferSize = FILESIZE / size;
    }

    n = bufferSize / sizeof(char);
    buffer = new char[n] {};

    // kreiramo izvedeni tip koji koristimo prilikom I/O operacija
    MPI_Type_contiguous(n, MPI_CHAR, &contigiousType);
    MPI_Type_commit(&contigiousType);

    // VAŽNO!!!! deo koda koji je potrebno otkomentarisati inicijalno radi popune fajla file1.dat
    //MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file1);
    //for (int i = 0; i < n; i++)
    //{
    //    buffer[i] = rank;
    //}
    //MPI_File_write_at_all(file1, rank * n, buffer, n, MPI_CHAR, MPI_STATUS_IGNORE);
    //MPI_File_close(&file1);

    //delete[] buffer;
    //buffer = new char[n] {};

    // otvaramo fajlove u odgovarajućim modovima
    MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file1);
    MPI_File_open(MPI_COMM_WORLD, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file2);

    // čitamo podatke tako da se ne zna koji proces odakle čita, kao što se traži u zadatku
    MPI_File_read_shared(file1, buffer, 1, contigiousType, MPI_STATUS_IGNORE);

    // kreiramo izvedeni tip koji nam služi za drugačiji file view prilikom upisa
    MPI_Type_vector(2, n / 2, n / 2 * size, MPI_CHAR, &writingViewType);
    MPI_Type_commit(&writingViewType);

    // postavljamo drugačiji fileview kao što se traži u zadatku (slika)
    MPI_File_set_view(file2, rank * n / 2 * sizeof(char), MPI_CHAR, writingViewType, "native", MPI_INFO_NULL);

    // vršimo upis u drugi fajl, postoji i neblokirajuća varijanta ove funkcije ali možda nije potrebna pošto se ništa
    // drugo ne izvršava u međuvremenu
    MPI_File_write_all(file2, buffer, 1, contigiousType, MPI_STATUS_IGNORE);
    //MPI_File_write_all_begin(file2, buffer, 1, contigiousType);
    //MPI_File_write_all_end(file2, buffer, MPI_STATUS_IGNORE);

    // kraj zadatka, oslobađamo memoriju i zatvaramo fajlove
    MPI_Type_free(&contigiousType);
    MPI_Type_free(&writingViewType);
    MPI_File_close(&file1);
    MPI_File_close(&file2);
    MPI_Finalize();

    delete[] buffer;
    return 0;
}

int aprilZadatak2A(int argc, char* argv[])
{
    // Napisati MPI program koji realizuje množenje matrice A dimenzija n x n i vektora B dimenzija n, čime se dobija
    // rezultujući vektor C dimenzija n, Matrica A i vektor B se inicijalizuju u master procesu. Broj procesa je p i 
    // uređeni su kao matrica q x q (q ^ 2 = p). Matrica A je podeljena u blokove i master proces distribuira 
    // odgovarajuće blokove matrice A po procesima kao što je prikazano na slici 1. za n = 8 i p = 16. Vektor b je 
    // distribuiran po procesima tako da proces Pi dobija elemente sa indeksima i % q, i % q + q, i % q + 2q, ......,
    // i % q + n - q. Predvideti da se slanje vrednosti bloka matrice A svakom procesu obavlja odjednom. Svaki proces
    // (uključujući i master proces) obavlja odgovarajuća izračunavanja i učestvuje u generisanju rezultata koji se 
    // prikazuje u procesu sadrži minimum svih vrednosti u matrici A. Predvideti da se slanje blokova matrice A svakom 
    // procesu obavlja sa po jednom naredbom MPI_Send kojom se šalje samo jedan izvedeni tip podatka. Slanje blokova 
    // vektora B i generisanje rezultata impelementirati korišćenjem grupnih operacija i funkcija za kreiranje novih
    // komunikatora.

    return 0;
}

int aprilZadatak2B(int argc, char* argv[])
{
    // Napisati MPI program koji kreira komunikator koji se sastoji od procesa na dijagonali u kvadratnoj mreži procesa.
    // Iz master procesa novog komunikatora poslati poruku svim ostalim procesima. Svaki proces novog komunikatora treba
    // da prikaže primljenu poruku.

    return 0;
}

int aprilZadatak3(int argc, char* argv[])
{
    // Napisati MPI Program koji manipuliše velikom količinom log informacija, tako što vrši paralelni upis i čitanje
    // binarne log datoteke. Log podaci nalaze se u datoteci file1.dat. Svi podaci vrše čitanje iste količine podataka,
    // tako da prvi proces čita podatke sa početka fajla, zatim drugi proces one u nastavku, itd. Upravo pročitane
    // podatke upisati u dve različite datoteke, prema sledećim zahtevima:
    // a) U datoteku file2.dat upisati podatke tako da se redosled upisa procesa ne može unapred predvideti.
    // b) U datoteku file3.dat upisati podatke tako da procesi, u redosledu od poslednjeg do prvog, upisuju 1/10 svojih
    // podataka, po round-robin principu.

    return 0;
}


//int zadatak1(int argc, char* argv[])
//{
//    const int n = 10;
//
//    int my_rank;
//    float A[n][n] = { 0 };
//    MPI_Status status;
//    MPI_Datatype column_mpi_t;
//    int i, j;
//    MPI_Init(&argc, &argv);
//    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
//    MPI_Type_vector(n, 1, n, MPI_FLOAT, &column_mpi_t);
//    MPI_Type_commit(&column_mpi_t);
//
//    if (my_rank == 0)
//    {
//        for (i = 0; i < n; i++)
//            for (j = 0; j < n; j++)
//                A[i][j] = (float)i;
//
//        MPI_Send(&(A[0][0]), 1, column_mpi_t, 1, 0, MPI_COMM_WORLD);
//    }
//    else
//    {
//        for (i = 0; i < n; i++)
//            for (j = 0; j < n; j++)
//                A[i][j] = 0;
//
//        MPI_Recv(&(A[0][0]), n, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
//
//        for (i = 0; i < n; i++)
//            printf("%3.1f", A[0][i]);
//        printf("\n");
//    }
//
//    MPI_Finalize();
//    return 0;
//}
//
//int zadatak2(int argc, char* argv[])
//{
//    const int n = 10;
//
//    int p;
//    int my_rank;
//    float A[n][n] = { 0 };
//    float T[n][n] = { 0 };
//    int displacements[n] = { 0 };
//    int block_lengths[n] = { 0 };
//    MPI_Datatype index_mpi_t;
//    int i, j;
//    MPI_Status status;
//    
//    MPI_Init(&argc, &argv);
//    MPI_Comm_size(MPI_COMM_WORLD, &p);
//    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
//    for (i = 0; i < n; i++)
//    {
//        block_lengths[i] = n - i;
//        displacements[i] = (n + 1) * i;
//    }
//    MPI_Type_indexed(n, block_lengths, displacements, MPI_FLOAT, &index_mpi_t);
//    MPI_Type_commit(&index_mpi_t);
//
//    if (my_rank == 0)
//    {
//        for (i = 0; i < n; i++)
//            for (j = 0; j < n; j++)
//                A[i][j] = (float)i + j;
//        MPI_Send(A, 1, index_mpi_t, 1, 0, MPI_COMM_WORLD);
//    }
//    else
//    {
//        for (i = 0; i < n; i++)
//            for (j = 0; j < n; j++)
//                T[i][j] = (float)0;
//        MPI_Recv(T, 1, index_mpi_t, 0, 0, MPI_COMM_WORLD, &status);
//
//        for (i = 0; i < n; i++)
//        {
//            for (j = 0; j < n; j++)
//                printf("%5.1f", T[i][j]);
//            printf("\n");
//        }
//    }
// 
//    MPI_Finalize();
//    return 0;
//}
//
//int zadatak3(int argc, char *argv[])
//{
//    //int items;
//
//    int rang;
//    struct { int a; double b; } value = { 0, 0.0 };
//    MPI_Datatype mystruct;
//    int blocklens[2] = { 0 };
//    MPI_Aint indices[2] = { 0 };
//    MPI_Datatype old_types[2] = { 0 };
//
//    MPI_Init(&argc, &argv);
//    MPI_Comm_rank(MPI_COMM_WORLD, &rang);
//    blocklens[0] = 1;
//    blocklens[1] = 1;
//    old_types[0] = MPI_INT;
//    old_types[1] = MPI_DOUBLE;
//
//    // MPI_Address(&value.a, &indices[0]);
//    // MPI_Address(&value.b, &indices[1]);
//    MPI_Get_address(&value.a, &indices[0]);
//    MPI_Get_address(&value.b, &indices[1]);
//
//    indices[1] = indices[1] - indices[0];
//    indices[0] = 0;
//
//    //MPI_Type_struct(2, blocklens, indices, old_types, &mystruct);
//    MPI_Type_create_struct(2, blocklens, indices, old_types, &mystruct);
//    MPI_Type_commit(&mystruct);
//
//    if (rang == 0)
//    {
//        std::cin >> value.a;
//        std::cin >> value.b;
//    }
//
//    MPI_Bcast(&value, 1, mystruct, 0, MPI_COMM_WORLD);
//    printf("Process %d got %d and %lf!\n", rang, value.a, value.b);
//    
//    MPI_Finalize();
//    return 0;
//}
//
//int zadatak4(int argc, char *argv[])
//{
//    const int m = 2;
//    const int n = 3;
//    const int k = 4;
//
//    int a[m][n] = { 0 };
//    int b[n][k] = { 0 };
//    int c[m][k] = { 0 };
//    int local_c[m][k] = { 0 };
//    int niza[m];
//    int nizb[k];
//    int rank;
//    int i, j;
//    int p;
//    MPI_Datatype vector, column;
//
//    MPI_Init(&argc, &argv);
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    MPI_Comm_size(MPI_COMM_WORLD, &p);
//    MPI_Type_vector(m, 1, n, MPI_INT, &vector);
//    MPI_Type_commit(&vector);
//    MPI_Type_create_resized(vector, 0, 1 * sizeof(int), &column);
//    MPI_Type_commit(&column);
//
//    if (rank == 0)
//    {
//        for (i = 0; i < m; i++)
//            for (j = 0; j < n; j++)
//                a[i][j] = i + j;
//
//        for (i = 0; i < n; i++)
//            for (j = 0; j < k; j++)
//                b[i][j] = 1 + j - i;
//    }
//    MPI_Scatter(a, 1, column, niza, m, MPI_INT, 0, MPI_COMM_WORLD);
//    MPI_Scatter(b, k, MPI_INT, nizb, k, MPI_INT, 0, MPI_COMM_WORLD);
//
//    for (i = 0; i < m; i++)
//        for (j = 0; j < k; j++)
//            local_c[i][j] = niza[i] * nizb[j];
//
//    for (i = 0; i < m; i++)
//        for (j = 0; j < k; j++)
//            c[i][j] = 0;
//
//    MPI_Reduce(&local_c[0][0], &c[0][0], m * k, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
//
//    if (rank == 0)
//    {
//        for (i = 0; i < m; i++)
//        {
//            for (j = 0; j < k; j++)
//                printf("%5d", c[i][j]);
//            printf("\n");
//        }
//    }
//
//    MPI_Finalize();
//    return 0;
//}
//
//int zadatak5(int argc, char* argv[])
//{
//    MPI_Group group_world;
//    MPI_Group odd_group;
//    MPI_Group even_group;
//    int i, p;
//    int n_even;
//    int n_odd;
//    int members[20] = { 0 };
//    int group_rank1;
//    int group_rank2;
//    int rank;
//
//    MPI_Init(&argc, &argv);
//    MPI_Comm_size(MPI_COMM_WORLD, &p);
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    MPI_Comm_group(MPI_COMM_WORLD, &group_world);
//
//    n_even = (p + 1) / 2;
//    n_odd = p - n_even;
//
//    for (i = 0; i < n_even; i++)
//        members[i] = 2 * i;
//
//    MPI_Group_incl(group_world, n_even, members, &even_group);
//    MPI_Group_rank(even_group, &group_rank1);
//    MPI_Group_excl(group_world, n_even, members, &odd_group);
//    MPI_Group_rank(odd_group, &group_rank2);
//    printf("Moj rank je: %d, moj even rank je %d, i moj odd rank je %d!\n", rank, group_rank1, group_rank2);
//
//    MPI_Finalize();
//    return 0;
//}
//
//int zadatak6(int argc, char* argv[])
//{
//    int mcol, irow, jcol, p;
//    MPI_Comm row_comm;
//    MPI_Comm col_comm;
//    MPI_Comm comm_2D;
//
//    int Iam;
//    int row_id;
//    int col_id;
//
//    mcol = 2;
//
//    MPI_Init(&argc, &argv);
//    MPI_Comm_rank(MPI_COMM_WORLD, &Iam);
//    MPI_Comm_size(MPI_COMM_WORLD, &p);
//
//    irow = Iam / mcol;
//    jcol = Iam % mcol;
//
//    comm_2D = MPI_COMM_WORLD;
//
//    MPI_Comm_split(comm_2D, irow, jcol, &row_comm);
//    MPI_Comm_split(comm_2D, jcol, irow, &col_comm);
//
//    MPI_Comm_rank(row_comm, &row_id);
//    MPI_Comm_rank(col_comm, &col_id);
//
//    printf("%8d %8d %8d %8d %8d\n", Iam, irow, jcol, row_id, col_id);
//
//    MPI_Finalize();
//    return 0;
//}
//
//int zadatak7(int argc, char* argv[])
//{
//    int rank;
//    int size;
//    int m_rank;
//
//    const int n = 3;
//    int a[n][n];
//    int b[n][n];
//    int c[n][n];
//    int row[n];
//    int column[n];
//    int tmp[n][n];
//    int rez[n][n];
//
//    int root = 0;
//    int no[1] = { 0 };
//    int br = 1;
//
//    int i;
//    int j;
//
//    MPI_Status status;
//    MPI_Datatype vector;
//    MPI_Group mat;
//    MPI_Group world;
//    MPI_Comm comm;
//
//    MPI_Init(&argc, &argv);
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    MPI_Comm_size(MPI_COMM_WORLD, &size);
//
//    MPI_Type_vector(n, 1, n, MPI_INT, &vector);
//    MPI_Type_commit(&vector);
//
//    MPI_Comm_group(MPI_COMM_WORLD, &world);
//
//    MPI_Group_excl(world, br, no, &mat);
//    MPI_Comm_create(MPI_COMM_WORLD, mat, &comm);
//
//    MPI_Group_rank(mat, &m_rank);
//
//    if (rank == root)
//    {
//        for (i = 0; i < n; i++)
//        {
//            for (j = 0; j < n; j++)
//            {
//                a[i][j] = 3 * i + j + 1;
//                b[i][j] = i + 1;
//            }
//        }
//
//        for (i = 0; i < n; i++)
//        {
//            MPI_Send(&a[0][i], 1, vector, i + 1, 33, MPI_COMM_WORLD);
//            MPI_Send(&b[i][0], n, MPI_INT, i + 1, 32, MPI_COMM_WORLD);
//        }
//    }
//    else
//    {
//        MPI_Recv(&column[0], n, MPI_INT, root, 33, MPI_COMM_WORLD, &status);
//        MPI_Recv(&row[0], n, MPI_INT, root, 32, MPI_COMM_WORLD, &status);
//
//        for (i = 0; i < n; i++)
//            for (j = 0; j < n; j++)
//                tmp[i][j] = column[i] * row[j];
//
//        MPI_Reduce(&tmp, &rez, n * n, MPI_INT, MPI_SUM, root, comm);
//    }
//
//    if (rank == root)
//        for (i = 0; i < n; i++)
//        {
//            for (j = 0; j < n; j++)
//                printf("%d ", rez[i][j]);
//            printf("\n");
//        }
//
//    MPI_Finalize();
//    return 0;
//}
//
//int zadatak8(int argc, char* argv[])
//{
//    constexpr int n = 6;
//
//    int p;
//    int q;
//    int k;
//
//    int irow;
//    int jcol;
//    int i;
//    int j;
//
//    int rank;
//    int row_id;
//    int col_id;
//
//    int a[n][n];
//    int b[n];
//    int c[n];
//
//    MPI_Datatype vrblok;
//    MPI_Status status;
//    MPI_Comm row_comm;
//    MPI_Comm col_comm;
//    MPI_Comm comm;
//
//    MPI_Init(&argc, &argv);
//
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    MPI_Comm_size(MPI_COMM_WORLD, &p);
//
//    q = (int)sqrt((double)p);
//    k = n / q;
//
//    int* local_a = (int*)calloc(k * k, sizeof(int));
//    int* local_b = (int*)calloc(k, sizeof(int));
//
//    if (local_a == NULL || local_b == NULL)
//    {
//        printf("calloc memory error!\n");
//        return 1;
//    }
//
//    MPI_Type_vector(k, k, n, MPI_INT, &vrblok);
//    MPI_Type_commit(&vrblok);
//
//    if (rank == 0)
//    {
//        for (i = 0; i < n; i++)
//        {
//            for (j = 0; j < n; j++)
//            {
//                a[i][j] = i + j;
//            }  
//        }
//        for (i = 0; i < n; i++)
//        {
//            b[i] = 1;
//        }
//    }
//    if (rank == 0)
//    {
//        int y = 0;
//        for (i = 0; i < k; i++)
//        {
//            for (j = 0; j < k; j++)
//            {
//                local_a[y++] = a[i][j];
//            }
//                
//        }
//            
//        int l = 1;
//        for (i = 0; i < q; i++)
//        {
//            for (j = 0; j < q; j++)
//            {
//                if ((i + j) != 0)
//                {
//                    MPI_Send(&a[i * k][j * k], 1, vrblok, l, 2, MPI_COMM_WORLD);
//                    l++;
//                }
//            }
// 
//        }               
//    }
//    else
//    {
//        MPI_Recv(local_a, k * k, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
//    }
//
//    irow = rank / q;
//    jcol = rank % q;
//    comm = MPI_COMM_WORLD;
//
//    MPI_Comm_split(comm, irow, jcol, &row_comm);
//    MPI_Comm_split(comm, jcol, irow, &col_comm);
//    MPI_Comm_rank(row_comm, &row_id);
//    MPI_Comm_rank(col_comm, &col_id);
//
//    if (col_id == 0)
//    {
//        MPI_Scatter(b, k, MPI_INT, local_b, k, MPI_INT, 0, row_comm);
//    }
//        
//
//    MPI_Bcast(local_b, k, MPI_INT, 0, col_comm);
//
//    int* MyResult = (int*)malloc(k * sizeof(int));
//    int* Result = (int*)malloc(n * sizeof(int));
//    int index = 0;
//
//    for (i = 0; i < k; i++)
//    {
//        MyResult[i] = 0;
//        for (j = 0; j < k; j++)
//            MyResult[i] += local_a[index++] * local_b[j];
//    }
//
//    MPI_Gather(MyResult, k, MPI_INT, Result, k, MPI_INT, 0, col_comm);
//
//    if (col_id == 0)
//    {
//        MPI_Reduce(Result, c, n, MPI_INT, MPI_SUM, 0, row_comm);
//    }
//
//    if (rank == 0)
//    {
//        for (i = 0; i < n; i++)
//        {
//            printf("c[%d] = %d ", i, c[i]);
//        } 
//        printf("\n");
//    }
//
//    free(local_a);
//    free(local_b);
//    free(MyResult);
//    free(Result);
//
//    MPI_Finalize();
//    return 0;
//}
//
//int zadatak9(int argc, char* argv[])
//{
//    /*const int SIZE = 12;
//    const int UP = 0;
//    const int DOWN = 1;
//
//    int numtasks;
//    int rank;
//    int source;
//    int dest;
//    int outbuf;
//    int i;
//    int tag = 1;
//    int nbrs[4];
//    int dims[2] = { 3, 4 };
//    int periods[2] = { 1,0 };
//    int reorder = 0;
//    int coords[2];
//
//    MPI_Comm cartcomm;
//    MPI_Status st;
//
//    MPI_Init(&argc, &argv);
//
//    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
//    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cartcomm);
//    
//    MPI_Comm_rank(cartcomm, &rank);
//
//    MPI_Cart_coords(cartcomm, rank, 2, coords);
//    MPI_Cart_shift(cartcomm, 0, coords[1], &nbrs[UP], &nbrs[DOWN]);
//
//    outbuf = rank;
//    dest = nbrs[1];
//    source = nbrs[0];
//
//    MPI_Sendrecv_replace(&outbuf, 1, MPI_INT, dest, 0, source, 0, cartcomm, &st);*/
//    return 0;
//}
//
//int zadatak10(int argc, char* argv[])
//{
//    const int FILESIZE = 1048576;
//    const int INTS_PER_BLK = 16;
//
//    int* buf;
//    int rank;
//    int nprocs;
//    int nints;
//    int bufsize;
//    MPI_File fh;
//    MPI_Datatype filetype;
//
//    MPI_Init(&argc, &argv);
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
//
//    bufsize = FILESIZE / nprocs;
//    buf = (int*)malloc(bufsize);
//    nints = bufsize / sizeof(int);
//
//    MPI_File_open(MPI_COMM_WORLD, "/pfs/datafile", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
//
//    MPI_Type_vector(nints / INTS_PER_BLK, INTS_PER_BLK, INTS_PER_BLK * nprocs, MPI_INT, &filetype);
//    MPI_Type_commit(&filetype);
//
//    MPI_File_set_view(fh, INTS_PER_BLK * sizeof(int) * rank, MPI_INT, filetype, "native", MPI_INFO_NULL);
//
//    MPI_File_read_all(fh, buf, nints, MPI_INT, MPI_STATUS_IGNORE);
//
//    MPI_File_close(&fh);
//
//    MPI_Type_free(&filetype);
//    free(buf);
//    MPI_Finalize();
//    return 0;
//}
//
//int april2020_zadatak3(int argc, char* argv[])
//{
//	//ima greške...
//
//	const int m = 6;
//	const int n = 3;
//	const int k = 4;
//
//	int rank, size;
//	int A[m][n] = { 0 };
//	int B[n][k] = { 0 };
//	int C[m][k] = { 0 };
//	int localMin = 99999;
//	struct { int rank = 0; int value = 0; } localMinStruct, finalMinStruct;
//	MPI_Datatype rowsType, rowsType_resized;
//
//	MPI_Init(&argc, &argv);
//	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//	MPI_Comm_size(MPI_COMM_WORLD, &size);
//
//	if (rank == 0)
//	{
//		for (int i = 0; i < m; i++)
//		{
//			for (int j = 0; j < n; j++)
//			{
//				A[i][j] = i * n + j;
//				printf("%4d ", A[i][j]);
//			}
//			printf("\n");
//		}
//		printf("\n");
//		for (int i = 0; i < n; i++)
//		{
//			for (int j = 0; j < k; j++)
//			{
//				B[i][j] = 2 * i + j;
//				printf("%4d ", B[i][j]);
//			}
//			printf("\n");
//		}
//	}
//
//	MPI_Bcast(B, n * k, MPI_INT, 0, MPI_COMM_WORLD);
//
//	MPI_Type_vector(m / size, n, n * size, MPI_INT, &rowsType);
//	MPI_Type_commit(&rowsType);
//	MPI_Type_create_resized(rowsType, 0, n * sizeof(int), &rowsType_resized);
//	MPI_Type_commit(&rowsType_resized);
//
//	int* localA = (int*)calloc(m / size * n, sizeof(int));
//	MPI_Scatter(A, 1, rowsType_resized, localA, m / size * n, MPI_INT, 0, MPI_COMM_WORLD);
//
//	printf("Ja sam rank %d\n", rank);
//	for (int i = 0; i < m / size; i++)
//	{
//		for (int j = 0; j < n; j++)
//		{
//			printf("%4d ", localA[i * n + j]);
//		}
//		printf("\n");
//	}
//
//	for (int i = 0; i < m / size; i++)
//	{
//		for (int j = 0; j < n; j++) {
//			if (localA[i * n + j] < localMin)
//				localMin = localA[i * n + j];
//		}
//	}
//
//	localMinStruct.rank = rank;
//	localMinStruct.value = localMin;
//
//	MPI_Reduce(&localMinStruct, &finalMinStruct, 1, MPI_DOUBLE_INT, MPI_MINLOC, 0, MPI_COMM_WORLD);
//
//	if (rank == 0)
//		printf("Final min %d je u ranku %d\n", finalMinStruct.value, finalMinStruct.rank);
//
//	int* localC = (int*)calloc(m / size * n, sizeof(int));
//
//	printf("LOCAL RESULT RANK - %d\n", rank);
//	for (int i = 0; i < m / size; i++)
//	{
//		for (int j = 0; j < n; j++)
//		{
//			localC[i * m / size + j] = localA[i * m / size + j] * B[i][j];
//			printf("%4d ", localC[i * m / size + j]);
//		}
//	}
//
//	MPI_Bcast(&finalMinStruct, 1, MPI_DOUBLE_INT, 0, MPI_COMM_WORLD);
//
//	MPI_Gather(localC, 1, rowsType_resized, &C[rank][0], 1, rowsType_resized, finalMinStruct.rank, MPI_COMM_WORLD);
//
//	if (rank == finalMinStruct.rank)
//	{
//		printf("FINAL! \n");
//		for (int i = 0; i < m; i++)
//		{
//			for (int j = 0; j < k; j++)
//			{
//				printf("%4d ", C[i][j]);
//			}
//			printf("\n");
//		}
//	}
//
//	free(localA);
//	free(localC);
//	MPI_Finalize();
//	return 0;
//}