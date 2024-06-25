#include "jun2024.h"

static struct double_int
{
	double value;
	int rank;
};
static int mpiWrite(int argc, char* argv[]);

int jun2024zadatak1(int argc, char* argv[])
{
	// Napisati MPI program koji će za niz realnih brojeva A računati vrednost M po sledećoj formuli:
	// M = SUMA (i = 1, N) ((A[i] + As) / C)
	// gde je As srednja vrednost elemenata niza A. Parametar C nalazi se u master procesu, dok se niz A nalazi u fajlu
	// "niz.dat". Svi procesi učestvuju u izračunavanju, i učitavaju deo niza A (smatrati da je dimenzija niza N deljiva
	// brojem procesa). Redosled učitavanja elemenata nije definisan, tj. svaki proces može pročitati bilo koji element
	// niza. Paralelizovati sve moguće operacije. Rezultat upisati u proces koji je učitao najveći element niza A.

	// Napomena: zadatak pokrenuti sa n procesa; Gde je 100 deljivo sa n.

	int rank;							// Rang procesora u komunikatoru
	int p;								// Broj procesa koji izvršavaju program

	constexpr int N = 100;              // Broj elemenata niza koji se čitaju iz fajla
	int count;							// Broj elemenata niza koje čita svaki proces
	double* A;                          // Deo glavnog niza a koji se učitava iz fajla

	double As;                          // Srednja vrednost niza A
	double C;                           // Proizvoljni parametar / konstanta
	double localM;						// Lokalni rezultat izračunavanja
	double M;                           // Krajnji rezultat

	double_int localMax;                // maksimalni element koji je proces pročitao iz fajla + id procesa                 
	double_int globalMax;					// globalni maksimum + id procesa

	double localSum;					// lokalni zbir elemenata pročitanih iz fajla
	double globalSum;					// globalni zbir svih elemenata pročitanih iz fajla

	MPI_File rFile;						// Fajl iz koga čitamo ulazne podatke
	
	// Init
	As = 0.0;
	C = 0.0;
	localM = 0.0;
	M = 0.0;
	localMax.value = std::numeric_limits<double>::min();
	localMax.rank = 0;
	globalMax.value = std::numeric_limits<double>::min();
	globalMax.rank = 0;
	localSum = 0.0;
	globalSum = 0.0;

	// Inicijalizacija fajla potrebnog potrebe zadatka - pokrenuti barem jednom
	// mpiWrite(argc, argv);

	// MPI Init
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	localMax.rank = rank;
	count = N / p;
	A = new double[count] {};

	// MPI čitanje iz fajla
	MPI_File_open(MPI_COMM_WORLD, "jun2024.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &rFile);
	// MPI_File_read_all(rFile, A, count, MPI_DOUBLE, MPI_STATUS_IGNORE);
	// MPI_File_read_ordered(rFile, A, count, MPI_DOUBLE, MPI_STATUS_IGNORE);
	MPI_File_read_shared(rFile, A, count, MPI_DOUBLE, MPI_STATUS_IGNORE);
	MPI_File_close(&rFile);

	// Računanje srednje vrednosti -> proces 0 prosleđuje srednju vrednost svima
	for (auto i = 0; i < count; i++)
	{
		localSum += A[i];
	}
	MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	if (rank == 0)
	{
		As = (double)globalSum / (double)N;
	}
	MPI_Bcast(&As, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// Proces 0 distribuira konstantu C ostalim procesima; Može i rand itd... ali nebitno...
	if (rank == 0)
	{
		C = 3.98;
	}
	MPI_Bcast(&C, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// Procesi pronalaze svoju lokalnu max vrednost i kasnije se distrubira max vrednost globalno
	for (auto i = 0; i < count; i++)
	{
		if (A[i] > localMax.value) localMax.value = A[i];
	}
	MPI_Reduce(&localMax, &globalMax, 1, MPI_DOUBLE_INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(&globalMax, 2, MPI_DOUBLE_INT, 0, MPI_COMM_WORLD);

	// Procesi vrše izračunavanje lokalno i kasnije šalju procesu koji ima max element iz učitanog niza.
	for (auto i = 0; i < count; i++)
	{
		localM += (double)(A[i] + As) / (double)C;
	}

	// Finalni rezultat M se upisuje u memoriju procesa sa max elementom iz inicijalnog niza
	MPI_Reduce(&localM, &M, 1, MPI_DOUBLE, MPI_SUM, globalMax.rank, MPI_COMM_WORLD);
	if (rank == globalMax.rank)
	{	
		std::cout << "Rezulat operacije je M = " << M << ", i nalazi se u procesu: " << rank;
		std::cout << ", sa maksimalnim elementom: " << localMax.value << std::endl;
	}
	
	// kraj zadatka, oslobađamo memoriju
	MPI_Finalize();
	delete[] A;

	return 0;
}

// Helper metoda za upis u fajl / generisanje fajla potrebnog za zadatak.
static int mpiWrite(int argc, char* argv[])
{
	constexpr int arr_size = 100;
	double array[arr_size]{};
	int rank;
	MPI_File fh;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{
		for (int i = 0; i < arr_size; i++)
		{
			array[i] = (double)i + 2.4;
		}

		MPI_File_open(MPI_COMM_WORLD, "jun2024.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
		MPI_File_write(fh, array, arr_size, MPI_DOUBLE, MPI_STATUS_IGNORE);
		MPI_File_close(&fh);
	}

	MPI_Finalize();

	return 0;
}