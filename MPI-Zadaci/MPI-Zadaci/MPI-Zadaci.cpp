// 1. instalirati msmpisetup i msmpisdk(naći na guglu majkrosoftov sajt...)
// 2. instalirati cpp desktop development pack u vs2022 installer
// 3. kreirati cpp console projekat(solution u istom folderu ? ? )
// 4. u glavni cpp #include "mpi.h"	
// 5. project->properties->c / c++->additional include directiories->dodajemo putanju gde je mpi sdk include folder
// 6. project->properties->linker->additional library directories->dodajemo putanju do mpi sdk Lib\x86
// 7. project->properties->Linker->Input->Additional dependencies->ukucamo msmpi.lib
// 8. apply & build proj
// 9. pozicioniramo se u debug folder otvaramo cmd i kucamo mpiexec - n 5 MPI12.exe
// 10. program iz tutoriala piše n = 5 puta hello world na ekran zato ovo iznad i.exe je ime našeg proj

#include "MPI-Zadaci.h"

int main(int argc, char* argv[])
{
    //ZADACI SA PREZENTACIJA
    //zadatak1(argc, argv);
    //zadatak2(argc, argv);
    //zadatak3(argc, argv);
    //zadatak4(argc, argv);
    //zadatak5(argc, argv);
    //zadatak6(argc, argv);

    //ISPITNI ZADACI
    //april2020_zadatak3(argc, argv);

    zadatak8(argc, argv);

}


int zadatak1(int argc, char* argv[])
{
    const int n = 10;

    int my_rank;
    float A[n][n] = { 0 };
    MPI_Status status;
    MPI_Datatype column_mpi_t;
    int i, j;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Type_vector(n, 1, n, MPI_FLOAT, &column_mpi_t);
    MPI_Type_commit(&column_mpi_t);

    if (my_rank == 0)
    {
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
                A[i][j] = (float)i;

        MPI_Send(&(A[0][0]), 1, column_mpi_t, 1, 0, MPI_COMM_WORLD);
    }
    else
    {
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
                A[i][j] = 0;

        MPI_Recv(&(A[0][0]), n, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);

        for (i = 0; i < n; i++)
            printf("%3.1f", A[0][i]);
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}

int zadatak2(int argc, char* argv[])
{
    const int n = 10;

    int p;
    int my_rank;
    float A[n][n] = { 0 };
    float T[n][n] = { 0 };
    int displacements[n] = { 0 };
    int block_lengths[n] = { 0 };
    MPI_Datatype index_mpi_t;
    int i, j;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    for (i = 0; i < n; i++)
    {
        block_lengths[i] = n - i;
        displacements[i] = (n + 1) * i;
    }
    MPI_Type_indexed(n, block_lengths, displacements, MPI_FLOAT, &index_mpi_t);
    MPI_Type_commit(&index_mpi_t);

    if (my_rank == 0)
    {
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
                A[i][j] = (float)i + j;
        MPI_Send(A, 1, index_mpi_t, 1, 0, MPI_COMM_WORLD);
    }
    else
    {
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
                T[i][j] = (float)0;
        MPI_Recv(T, 1, index_mpi_t, 0, 0, MPI_COMM_WORLD, &status);

        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
                printf("%5.1f", T[i][j]);
            printf("\n");
        }
    }
 
    MPI_Finalize();
    return 0;
}

int zadatak3(int argc, char *argv[])
{
    //int items;

    int rang;
    struct { int a; double b; } value = { 0, 0.0 };
    MPI_Datatype mystruct;
    int blocklens[2] = { 0 };
    MPI_Aint indices[2] = { 0 };
    MPI_Datatype old_types[2] = { 0 };

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rang);
    blocklens[0] = 1;
    blocklens[1] = 1;
    old_types[0] = MPI_INT;
    old_types[1] = MPI_DOUBLE;

    // MPI_Address(&value.a, &indices[0]);
    // MPI_Address(&value.b, &indices[1]);
    MPI_Get_address(&value.a, &indices[0]);
    MPI_Get_address(&value.b, &indices[1]);

    indices[1] = indices[1] - indices[0];
    indices[0] = 0;

    //MPI_Type_struct(2, blocklens, indices, old_types, &mystruct);
    MPI_Type_create_struct(2, blocklens, indices, old_types, &mystruct);
    MPI_Type_commit(&mystruct);

    if (rang == 0)
    {
        std::cin >> value.a;
        std::cin >> value.b;
    }

    MPI_Bcast(&value, 1, mystruct, 0, MPI_COMM_WORLD);
    printf("Process %d got %d and %lf!\n", rang, value.a, value.b);
    
    MPI_Finalize();
    return 0;
}

int zadatak4(int argc, char *argv[])
{
    const int m = 2;
    const int n = 3;
    const int k = 4;

    int a[m][n] = { 0 };
    int b[n][k] = { 0 };
    int c[m][k] = { 0 };
    int local_c[m][k] = { 0 };
    int niza[m];
    int nizb[k];
    int rank;
    int i, j;
    int p;
    MPI_Datatype vector, column;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Type_vector(m, 1, n, MPI_INT, &vector);
    MPI_Type_commit(&vector);
    MPI_Type_create_resized(vector, 0, 1 * sizeof(int), &column);
    MPI_Type_commit(&column);

    if (rank == 0)
    {
        for (i = 0; i < m; i++)
            for (j = 0; j < n; j++)
                a[i][j] = i + j;

        for (i = 0; i < n; i++)
            for (j = 0; j < k; j++)
                b[i][j] = 1 + j - i;
    }
    MPI_Scatter(a, 1, column, niza, m, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(b, k, MPI_INT, nizb, k, MPI_INT, 0, MPI_COMM_WORLD);

    for (i = 0; i < m; i++)
        for (j = 0; j < k; j++)
            local_c[i][j] = niza[i] * nizb[j];

    for (i = 0; i < m; i++)
        for (j = 0; j < k; j++)
            c[i][j] = 0;

    MPI_Reduce(&local_c[0][0], &c[0][0], m * k, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        for (i = 0; i < m; i++)
        {
            for (j = 0; j < k; j++)
                printf("%5d", c[i][j]);
            printf("\n");
        }
    }

    MPI_Finalize();
    return 0;
}

int zadatak5(int argc, char* argv[])
{
    MPI_Group group_world;
    MPI_Group odd_group;
    MPI_Group even_group;
    int i, p;
    int n_even;
    int n_odd;
    int members[20] = { 0 };
    int group_rank1;
    int group_rank2;
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_group(MPI_COMM_WORLD, &group_world);

    n_even = (p + 1) / 2;
    n_odd = p - n_even;

    for (i = 0; i < n_even; i++)
        members[i] = 2 * i;

    MPI_Group_incl(group_world, n_even, members, &even_group);
    MPI_Group_rank(even_group, &group_rank1);
    MPI_Group_excl(group_world, n_even, members, &odd_group);
    MPI_Group_rank(odd_group, &group_rank2);
    printf("Moj rank je: %d, moj even rank je %d, i moj odd rank je %d!\n", rank, group_rank1, group_rank2);

    MPI_Finalize();
    return 0;
}

int zadatak6(int argc, char* argv[])
{
    int mcol, irow, jcol, p;
    MPI_Comm row_comm;
    MPI_Comm col_comm;
    MPI_Comm comm_2D;

    int Iam;
    int row_id;
    int col_id;

    mcol = 2;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &Iam);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    irow = Iam / mcol;
    jcol = Iam % mcol;

    comm_2D = MPI_COMM_WORLD;

    MPI_Comm_split(comm_2D, irow, jcol, &row_comm);
    MPI_Comm_split(comm_2D, jcol, irow, &col_comm);

    MPI_Comm_rank(row_comm, &row_id);
    MPI_Comm_rank(col_comm, &col_id);

    printf("%8d %8d %8d %8d %8d\n", Iam, irow, jcol, row_id, col_id);

    MPI_Finalize();
    return 0;
}

int zadatak7(int& argc, char**& argv)
{
    int rank;
    int size;
    int m_rank;

    const int n = 3;
    int a[n][n];
    int b[n][n];
    int c[n][n];
    int row[n];
    int column[n];
    int tmp[n][n];
    int rez[n][n];

    int root = 0;
    int no[1] = { 0 };
    int br = 1;

    int i;
    int j;

    MPI_Status status;
    MPI_Datatype vector;
    MPI_Group mat;
    MPI_Group world;
    MPI_Comm comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Type_vector(n, 1, n, MPI_INT, &vector);
    MPI_Type_commit(&vector);

    MPI_Comm_group(MPI_COMM_WORLD, &world);

    MPI_Group_excl(world, br, no, &mat);
    MPI_Comm_create(MPI_COMM_WORLD, mat, &comm);

    MPI_Group_rank(mat, &m_rank);

    if (rank == root)
    {
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                a[i][j] = 3 * i + j + 1;
                b[i][j] = i + 1;
            }
        }

        for (i = 0; i < n; i++)
        {
            MPI_Send(&a[0][i], 1, vector, i + 1, 33, MPI_COMM_WORLD);
            MPI_Send(&b[i][0], n, MPI_INT, i + 1, 32, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(&column[0], n, MPI_INT, root, 33, MPI_COMM_WORLD, &status);
        MPI_Recv(&row[0], n, MPI_INT, root, 32, MPI_COMM_WORLD, &status);

        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
                tmp[i][j] = column[i] * row[j];

        MPI_Reduce(&tmp, &rez, n * n, MPI_INT, MPI_SUM, root, comm);
    }

    if (rank == root)
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
                printf("%d ", rez[i][j]);
            printf("\n");
        }

    MPI_Finalize();
    return 0;
}

int zadatak8(int& argc, char**& argv)
{





    return 0;
}





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