#ifndef __CUDACC__
#define __CUDACC__
#endif

#include "april2023_zadatak1.cuh"

// Anti-Aliasing je tehnika kojom se popravlja kvalitet slika niske rezolucije. Postoje različiti algoritmi koji
// rešavaju ovaj problem u zavisnosti od situacije. Neka je dat specifičan slučaj gde je, kao matrica nula i jedinica,
// data uvećana crno-bela slika koja predstavlja potpuno crno slovo (1) na potpuno belom papiru (0). Nad ovom slikom
// potrebno je primeniti jednostavnu Anti-Aliasing tehniku, koja preračunava nove boje svakog piksela kao srednju
// vrednost njegove boje i boje svih 8 susednih piksela (ako ne postoji 8 susednih, uzima se onoliko koliko ih ima).
// Proceduru treba ponoviti k puta. Koristeći CUDA tehnologiju, napisati paralelni program koji što efikasnije izvršava
// ovu transformaciju.

#define PICTURE_DIM 128
#define BLOCK_DIM 32 // ne preko 32 zbog deljene memorije, može da se radi veći blok ali bez deljene

__global__ void AntiAliasingKernel(float* pic_device_in, float* pic_device_out, int k)
{
	__shared__ float  pic_shared_step1[(BLOCK_DIM + 2) * (BLOCK_DIM + 2)];
	__shared__ float  pic_shared_step2[(BLOCK_DIM + 2) * (BLOCK_DIM + 2)];
	
	// Calculate global indices
	unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
	unsigned int idy = blockDim.y * blockIdx.y + threadIdx.y;

	// Calculate shared memory indices with padding
	unsigned int shared_index_x = threadIdx.x + 1;
	unsigned int shared_index_y = threadIdx.y + 1;

	// Calculate global index for current thread
	unsigned int index = idy * blockDim.x * gridDim.x + idx;

	// Load pixel data into shared memory
	pic_shared_step1[shared_index_y * (BLOCK_DIM + 2) + shared_index_x] = pic_device_in[index];

	// Edge loading - handle edges by loading additional pixels
	if (threadIdx.x == 0)
	{
		pic_shared_step1[shared_index_y * (BLOCK_DIM + 2)] = pic_device_in[index - 1]; // load left edge
	}
	else if (threadIdx.x == blockDim.x - 1)
	{
		pic_shared_step1[shared_index_y * (BLOCK_DIM + 2) + blockDim.x + 1] = pic_device_in[index + 1]; // load right edge
	}

	// Load top and bottom edges
	if (threadIdx.y == 0)
	{
		pic_shared_step1[shared_index_x] = pic_device_in[index - blockDim.x]; // load top edge
	}
	else if (threadIdx.y == blockDim.y - 1)
	{
		pic_shared_step1[(BLOCK_DIM + 2) * (BLOCK_DIM + 1) + shared_index_x] = pic_device_in[index + blockDim.x]; // load bottom edge
	}

	// Load corner pixels
	if (threadIdx.x == 0 && threadIdx.y == 0)
	{
		pic_shared_step1[0] = pic_device_in[index - blockDim.x - 1]; // top-left corner
	}
	else if (threadIdx.x == blockDim.x - 1 && threadIdx.y == 0)
	{
		pic_shared_step1[BLOCK_DIM + 1] = pic_device_in[index - blockDim.x + 1]; // top-right corner
	}
	else if (threadIdx.x == 0 && threadIdx.y == blockDim.y - 1)
	{
		pic_shared_step1[(BLOCK_DIM + 2) * (BLOCK_DIM + 1)] = pic_device_in[index + blockDim.x - 1]; // bottom-left corner
	}
	else if (threadIdx.x == blockDim.x -1 && threadIdx.y == blockDim.y - 1)
	{
		pic_shared_step1[(BLOCK_DIM + 2) * (BLOCK_DIM + 1) + BLOCK_DIM + 1] = pic_device_in[index + blockDim.x + 1]; // bottom-right corner
	}

	__syncthreads(); // snychronize threads after loading shared memory

	// Apply anti-aliasing K times
	for (int iter = 0; iter < k; iter++)
	{
		// calculate the new value of the current pixel based on the average of its neighbors
		float sum = 0.0f;
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				sum += pic_shared_step1[(shared_index_y + i) * (BLOCK_DIM + 2) + shared_index_x + j];
			}
		}

		float avg = sum / 9.0f; // average of 9 neighboring pixels

		// update the output image
		pic_shared_step2[shared_index_y * (BLOCK_DIM + 2) + shared_index_x] = avg;

		__syncthreads(); // sync threads after updating shared memory

		// fix the swap

		// swap shared memory arrays for the next iteration
		float* temp = pic_shared_step1;
		//pic_shared_step1 = pic_shared_step2;
		//pic_shared_step2 = temp;

	}

	// write the final result to global memory
	pic_device_out[index] = pic_shared_step1[shared_index_y * (BLOCK_DIM + 2) + shared_index_x];
}

static int AntiAliasing(float* pic_host_in, float* pic_host_out, int k)
{
	float* pic_device_in = nullptr;
	float* pic_device_out = nullptr;
	int size;
	dim3 block_count;
	dim3 block_size;

	size = static_cast<unsigned long long>(PICTURE_DIM) * PICTURE_DIM * sizeof(float);

	block_count.x = (int)ceil((float)PICTURE_DIM / (float)BLOCK_DIM);
	block_count.y = block_count.x;

	block_size.x = BLOCK_DIM;
	block_size.y = BLOCK_DIM;


	cudaMalloc((void**)&pic_device_in, size);
	cudaMalloc((void**)&pic_device_out, size);

	cudaMemcpy(pic_device_in, pic_host_in, size, cudaMemcpyHostToDevice);

	// kernel
	AntiAliasingKernel<<<block_count, block_size>>>(pic_device_in, pic_device_out, k);

	cudaMemcpy(pic_host_out, pic_device_out, size, cudaMemcpyDeviceToHost);

	cudaFree(pic_device_in);
	cudaFree(pic_device_out);

	cudaDeviceReset();

	return 0;
}

static int FillPictures(float* pic_a, float* pic_b)
{
	// random slovo F

	for (int i = 0; i < PICTURE_DIM; i++)
	{
		for (int j = 0; j < PICTURE_DIM; j++)
		{
			pic_a[i * PICTURE_DIM + j] = 0.0;
			pic_b[i * PICTURE_DIM + j] = 0.0;
		}
	}

	for (int i = 9; i < 90; i++)
	{
		for (int j = 29; j < 40; j++)
		{
			pic_a[i * PICTURE_DIM + j] = 1.0;
		}
	}

	for (int i = 9; i < 30; i++)
	{
		for (int j = 40; j < 60; j++)
		{
			pic_a[i * PICTURE_DIM + j] = 1.0;
		}
	}

	for (int i = 50; i < 70; i++)
	{
		for (int j = 40; j < 50; j++)
		{
			pic_a[i * PICTURE_DIM + j] = 1.0;
		}
	}

	pic_a[PICTURE_DIM * PICTURE_DIM - 3] = 7;
	pic_a[PICTURE_DIM * PICTURE_DIM - 4] = 7;
	pic_a[PICTURE_DIM * PICTURE_DIM - 5] = 7;
	pic_a[PICTURE_DIM * PICTURE_DIM - PICTURE_DIM - 3] = 7;

	return 0;
}

static int PrintOutSample(float* pic, int x_start, int y_start, int x_size, int y_size)
{
	if (x_start < 0 || y_start < 0 || x_size > PICTURE_DIM || y_size > PICTURE_DIM)
	{
		std::cout << "Error printing sample!\n";
		return 1;
	}

	for (int i = x_start; i < x_start + x_size; i++)
	{
		for (int j = y_start; j < y_start + y_size; j++)
		{
			std::cout << pic[i * PICTURE_DIM + j];
		}
		std::cout << std::endl;
	}

	return 0;
}

int april2023_zadatak1()
{
	float picture_a[PICTURE_DIM * PICTURE_DIM];
	float picture_b[PICTURE_DIM * PICTURE_DIM];

	FillPictures(picture_a, picture_b);

	AntiAliasing(picture_a, picture_b, 3);

	//TestResults(picture_a, picture_b);
	PrintOutSample(picture_a, 0, 0, PICTURE_DIM, PICTURE_DIM);

	std::cout << "------------------------" << std::endl;

	PrintOutSample(picture_b, 0, 0, PICTURE_DIM, PICTURE_DIM);

	return 0;
}
