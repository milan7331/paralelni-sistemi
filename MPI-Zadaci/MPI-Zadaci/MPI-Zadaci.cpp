#include "MPI-Zadaci.h"

int main(int argc, char* argv[])
{
    // ISPITNI ZADACI IZ 2023.
    // januar2023zadatak2A(argc, argv);
    // januar2023zadatak2B(argc, argv);
    // januar2023zadatak3(argc, argv);
	// april2023zadatak2A(argc, argv);
	// april2023zadatak2B(argc, argv);
	// april2023zadatak3(argc, argv);

    // ISPITNI ZADACI IZ 2024. 


    //ZADACI S RAČUNSKIH VEŽBI
    // zadatak1(argc, argv);
    // zadatak2(argc, argv);
    // zadatak3(argc, argv);
    // zadatak4(argc, argv);
    // zadatak5(argc, argv);
    // zadatak6(argc, argv);
    // zadatak8(argc, argv);

    return 0;
}

int april2020zadatak3(int argc, char* argv[])
{
	//ima greške...

	const int m = 6;
	const int n = 3;
	const int k = 4;

	int rank, size;
	int A[m][n] = { 0 };
	int B[n][k] = { 0 };
	int C[m][k] = { 0 };
	int localMin = 99999;
	struct { int rank = 0; int value = 0; } localMinStruct, finalMinStruct;
	MPI_Datatype rowsType, rowsType_resized;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == 0)
	{
		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < n; j++)
			{
				A[i][j] = i * n + j;
				printf("%4d ", A[i][j]);
			}
			printf("\n");
		}
		printf("\n");
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < k; j++)
			{
				B[i][j] = 2 * i + j;
				printf("%4d ", B[i][j]);
			}
			printf("\n");
		}
	}

	MPI_Bcast(B, n * k, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Type_vector(m / size, n, n * size, MPI_INT, &rowsType);
	MPI_Type_commit(&rowsType);
	MPI_Type_create_resized(rowsType, 0, n * sizeof(int), &rowsType_resized);
	MPI_Type_commit(&rowsType_resized);

	int* localA = (int*)calloc(m / size * n, sizeof(int));
	MPI_Scatter(A, 1, rowsType_resized, localA, m / size * n, MPI_INT, 0, MPI_COMM_WORLD);

	printf("Ja sam rank %d\n", rank);
	for (int i = 0; i < m / size; i++)
	{
		for (int j = 0; j < n; j++)
		{
			printf("%4d ", localA[i * n + j]);
		}
		printf("\n");
	}

	for (int i = 0; i < m / size; i++)
	{
		for (int j = 0; j < n; j++) {
			if (localA[i * n + j] < localMin)
				localMin = localA[i * n + j];
		}
	}

	localMinStruct.rank = rank;
	localMinStruct.value = localMin;

	MPI_Reduce(&localMinStruct, &finalMinStruct, 1, MPI_DOUBLE_INT, MPI_MINLOC, 0, MPI_COMM_WORLD);

	if (rank == 0)
		printf("Final min %d je u ranku %d\n", finalMinStruct.value, finalMinStruct.rank);

	int* localC = (int*)calloc(m / size * n, sizeof(int));

	printf("LOCAL RESULT RANK - %d\n", rank);
	for (int i = 0; i < m / size; i++)
	{
		for (int j = 0; j < n; j++)
		{
			localC[i * m / size + j] = localA[i * m / size + j] * B[i][j];
			printf("%4d ", localC[i * m / size + j]);
		}
	}

	MPI_Bcast(&finalMinStruct, 1, MPI_DOUBLE_INT, 0, MPI_COMM_WORLD);

	MPI_Gather(localC, 1, rowsType_resized, &C[rank][0], 1, rowsType_resized, finalMinStruct.rank, MPI_COMM_WORLD);

	if (rank == finalMinStruct.rank)
	{
		printf("FINAL! \n");
		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < k; j++)
			{
				printf("%4d ", C[i][j]);
			}
			printf("\n");
		}
	}

	free(localA);
	free(localC);
	MPI_Finalize();
	return 0;
}