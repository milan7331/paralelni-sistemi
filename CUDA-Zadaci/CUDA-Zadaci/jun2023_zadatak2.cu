#ifndef __CUDACC__
#define __CUDACC__
#endif

#include "jun2023zadatak2.cuh"

// Korišćenjem CUDA tehnologije, u programskom jeziku C/C++ napisati program koji nalazi minimalni element na glavnoj
// dijagonali kvadratne matrice A dimenzija n x n. Obratiti pažnju na efikasnost paralelizacije. Omogućiti pozivanje
// kernela za matrice proizvoljne veličine.

#define N 1940 // max 12228 zbog deljene memorije
#define BLK_SIZE 1024

__global__ void MinimalElementKernel(float* device_matrix, float* device_min_element, int* device_array_size)
{
	__shared__ float main_diagonal[BLK_SIZE];
	unsigned int index = blockDim.x * blockIdx.x + threadIdx.x;

	main_diagonal[threadIdx.x] = (index < N)? A_device[index * N + index] : FLT_MAX;

	__syncthreads();




}

static int MinimalElement(float* host_matrix, float* host_min_element)
{
	int size = N * N * sizeof(float);
	const int block_count = (int)ceil((float)N / (float)BLK_SIZE);

	float* device_matrix = nullptr;
	float* device_min_element = nullptr;
	float* device_array_size = nullptr;

	

	cudaMalloc((void**)&device_matrix, size);
	cudaMalloc((void**)&device_min_element, sizeof(float));
	cudaMalloc((void**)&device_array_size, sizeof(float));
	
	cudaMemcpy(device_matrix, host_matrix, size, cudaMemcpyHostToDevice);


	MinimalElementKernel<<<block_count, N>>>(A_device, min_element_device);

	cudaMemcpy(host_min_element, device_min_element, sizeof(float), cudaMemcpyDeviceToHost);

	cudaFree(A_device);
	cudaFree(min_element_device);

	cudaDeviceReset();

	return 0;
}

static int FillMatrix(float* A)
{
	constexpr float min = std::numeric_limits<float>::min();
	constexpr float max = std::numeric_limits<float>::max();
	constexpr float range = max - min;

	srand((unsigned int)time(0));

	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			A[i * N + j] = min + static_cast<float>(rand()) / (RAND_MAX / range);
		}
	}

	return 0;
}

static bool TestResult(float* A, float* min_element_returned)
{
	for (auto i = 0; i < N; i++)
	{
		if (A[i * N + i] < *min_element_returned)
		{
			return false;
		}
	}

	return true;
}

int jun2023zadatak2()
{
	float A[N * N];
	float min_element_host;

	FillMatrix(A);

	MinimalElement(A, &min_element_host);

	TestResult(A, &min_element_host);

	return 0;
}