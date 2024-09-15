#include "januar2023.h"

struct double_int                       // struktura koju koristimo za pronalazak maksimalne vrednosti
{
    double value = 0;                   // max vrednost
    int rank = 0;                       // lokacija - id procesa koji sadrži ovu vrednost
};

int januar2023zadatak2A(int argc, char* argv[])
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

    // Napomena: predviđeno je da se zadatak pokreće sa nProcs = 25.
    // Napomena: Maksimalnu vrednost verovatno ne treba računati kao ovde? Verovatno postoji jednostavnije rešenje.
   

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

    double_int localMax;                // struktura koja sadrži maksimalni element u procesu
    double_int globalMax;               // struktura koja sadrži globalni maksimum

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
    localMax.rank = rank;
    localMax.value = localC[0][0];

    for (int i = 0; i < K; i++)
    {
        for (int j = 0; j < K; j++)
        {
            if (localMax.value < localC[i][j])
            {
                localMax.value = localC[i][j];
            }
        }
    }

    // vraćamo max vrednost master procesu
    // napomena: postoji i drugi način da se ovo odradi tako što master procesu šaljemo samo max element i onda
    // broadcastujemo taj max element svim procesima -> ako proces sadrži globalni masksimalni element ispisuje rank
    // napomena: postoji i treći način da se ovo odradi, ujedno i najlakši -> master proces pronalazi master element i
    // onda na osnovu njegove lokacije u matrici zaključuje računicom kom procesu pripada (delimo sa q??).
    MPI_Reduce(&localMax, &globalMax, 1, MPI_DOUBLE_INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);

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
        std::cout << "Maksimalni element (" << globalMax.value << ")";
        std::cout << " sadrži proces: " << globalMax.rank << std::endl;
    }

    // kraj zadatka oslobađamo memoriju
    MPI_Type_free(&kRows);
    MPI_Type_free(&kCols);
    MPI_Type_free(&kColsResized);
    MPI_Finalize();

    return 0;
}

int januar2023zadatak2B(int argc, char* argv[])
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
    MPI_Datatype types[5]{};            // niz tipova koje struktura sadrzi
    int blocklens[5]{};                 // broj elemenata određenog tipa
    MPI_Aint base;
    MPI_Aint displacements[5]{};        // niz pomeraja svakog bloka

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
        std::cout << "Potrebno je program pokrenuti sa vecim brojem procesa!";
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
            std::cout << "Prosecna ocena: ";
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
        std::cout << "Proces 1 sadrzi sledece podatke: " << std::endl;
        for (int i = 0; i < N; i++)
        {
            std::cout << "Student broj: " << i + 1 << std::endl;
            std::cout << "Indeks: " << studenti[i].indeks << " ";
            std::cout << "Ime: " << studenti[i].ime << " ";
            std::cout << "Prezime: " << studenti[i].prezime << " ";
            std::cout << "Prosecna ocena: " << studenti[i].prosecnaOcena << " ";
            std::cout << "Godina studija: " << studenti[i].godinaStudija << " ";
            std::cout << std::endl;
        }
    }

    // kraj zadatka, oslobađamo memoriju
    MPI_Type_free(&studentType);
    MPI_Finalize();
    return 0;
}

int januar2023zadatak3(int argc, char* argv[])
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