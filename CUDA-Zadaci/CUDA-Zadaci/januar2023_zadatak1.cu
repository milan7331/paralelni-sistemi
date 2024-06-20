#ifndef __CUDACC__
#define __CUDACC__
#endif

#include "januar2023_zadatak1.cuh"

// Neka je dat CUDA kernel i odgovarajuća funkcija koja ga poziva.
// __global__ void vecAddKernel(float* A_d, float* B_d, float* C_d, int n)
// {
//     int i = threadIdx.x + blockDim.x * blockIdx.x;
//     if (i < n)
//     {
//         C_d[i] = A_d[i] + B_d[i];
//     }
// }
//   
// int vecAdd(float* A, float* B, float* C, int n) //pordrazumevati da su nizovi A,B i C dužine n
// {
//     int size = n * sizeof(float);
//     cudaMalloc((void**)&A_d, size);
//     cudaMalloc((void**)&B_d, size);
//     cudaMalloc((void**)&C_d, size);
//     cudaMemcpy(A_d, A, size, cudaMemcpyHostToDevice);
//     cudaMemcpy(B_d, B, size, cudaMemcpyHostToDevice);
//     vecAddKernel << <ceil(n / 256), 256 >> > (A_d, B_d, C_d, n);
//     cudaMemcpy(C, C_d, size, cudaMemcpyDeviceToHost);
// }
// 
// a.  Ako su A, B, C nizovi od 1000 elemenata, koliko će blokova niti biti pokrenuto?
// b.  Ako su A, B, C nizovi od 1000 elemenata, koliko će warp-ova biti u svakom od blokova?
// c.  Ako su A, B, C nizovi od 1000 elemenata, koliko će niti biti u gridu?
// d.  Ako su A, B, C nizovi od 1000 elemenata, da li će postojati divergentnost u izvršenju kernela? Ako da, u kojoj
//     liniji. Objasniti zašto.
// e.  Ako su A, B, C nizovi od 768 elemenata, da li će postojati divergentnost u izvršenju kernela? Ako da, u kojoj
//     liniji. Objasniti zašto.
// f.  Šta dati kernel radi?
// g.  Modifikovati kernel tako da svaki 256. element rezultujućeg niza sadrži srednju vrednost prethodnih 255
//     elemenata.
//
//     Napomena: kod će biti izmenjen u odnosu na originalni tako da je moguće pokrenuti aplikaciju.
//     Rešenje:
// 
// a.  Biće pokrenuto 4 bloka.
// b.  Ako uzmemo da blok ima max veličinu od 256 tredova, i da warp standardno čine 32 niti, dolazimo do zaključka da
//     jedan blok ima 8 warpova.
// c.  Broj niti u gridu je 256 * 4 = 1024
// d.  Da, do divergentnosti dolazi u liniji koja ima naredbu grananja. Konkretno u četvrtom bloku imamo 24 niti koje
//     neće raditi ništa, samo će čekati da ostale niti završe svoje operacije.
// e.  Ne, do divergentnosti može doći u liniji koja ima naredbu granaja. U našem slučaju sve niti će biti upošljene.
// f.  Svaka nit koja izvršava kernel računa index na osnovu koga pristupa nizovima. Indeksi su u odnosu na dimenzije
//     grida i bloka raspoređeni "linearno". Svaka nit je zadužena za samo jedan indeks, dva učitavanja i jedna operacija
//     sabiranja. Ove operacije se izvršavaju samo ako je indeks niti manji od veličine niza. Ostale niti ne rade ništa.
// g.  Rešenje je priloženo ispod.
// 

#define BLK_SIZE 256                    // broj tredova u jednom bloku

__global__ void vecAddKernel(float* A_d, float* B_d, float* C_d, int n)
{
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    if (i < n)
    {
        C_d[i] = A_d[i] + B_d[i];       // koristi se globalna memorija koja je spora
    }
    
}

__global__ void vecAddKernelModified(float* A_d, float* B_d, float* C_d, int n)
{
    __shared__ float C_temp[BLK_SIZE];  // koristimo deljivu memoriju na nivou bloka

    // inicijalizacija lokalne promenljive koja ima realnu lokaciju niti u odnosu na grid, korisno zbog pristupa nizu
    int index = blockDim.x * blockIdx.x + threadIdx.x;

    // inicijalno učitavanje podataka u privremenu deljenu memoriju
    if (index < n)
    {
        C_temp[threadIdx.x] = A_d[index] + B_d[index];
    }
    __syncthreads();

    // poslednja nit iz bloka računa srednju vrednost prethodnih niti. Ovo je jedini razlog zašto koristimo deljivu
    // memoriju, na ovaj način izbegavamo ponovno obraćanje globalnoj memoriji koja je znatno sporija.
    if (index < n && threadIdx.x + 1 == blockDim.x)
    {
        for (int i = 0; i < threadIdx.x; i++)
        {
            C_temp[threadIdx.x] += C_temp[i];
        }

        C_temp[threadIdx.x] /= 255;
    }
    __syncthreads();
    
    // na kraju upisujemo izmenjeni niz u globalnu memoriju
    if (index < n)
    {
        C_d[index] = C_temp[threadIdx.x];
    }
}

static int vecAdd(float* A, float* B, float* C, int n)
{
    // Napomena: funkcija je izmenjena da bi mogla da se pokrene ispravno.

    float* A_d = nullptr;               // pokazivač na niz A u memoriji device-a / grafičke kartice
    float* B_d = nullptr;               // pokazivač na niz B u memoriji device-a / grafičke kartice
    float* C_d = nullptr;               // pokazivač na niz C u memoriji device-a / grafičke kartice

    int size;                           // veličina niza u bajtovima
    int blockCount;                     // potreban broj blokova se računa u odnosu na broj elemenata zadatih nizova
                                        // zbog ograničenja broja tredova po bloku
        
    // inicijalizacija
    size = n * sizeof(float);
    blockCount = (int)ceil((float)n / (float)BLK_SIZE);

    // cuda alokacija memorije i prenos nizova u globalnu memoriju grafičke kartice
    cudaMalloc((void**)&A_d, size);
    cudaMalloc((void**)&B_d, size);
    cudaMalloc((void**)&C_d, size);
    cudaMemcpy(A_d, A, size, cudaMemcpyHostToDevice);
    cudaMemcpy(B_d, B, size, cudaMemcpyHostToDevice);

    // poziv samog kernela koji se izvršava na kartici
    vecAddKernelModified<<<blockCount, BLK_SIZE>>>(A_d, B_d, C_d, n);

    // kopiranje rezultata nazad u glavnu memoriju procesora
    cudaMemcpy(C, C_d, size, cudaMemcpyDeviceToHost);

    // oslobađanje memorije i reset grafičke kartice
    cudaFree(A_d);
    cudaFree(B_d);
    cudaFree(C_d);
    cudaDeviceReset();

    return 0;
}

static void TestResults(float* A, float* B, float* C, int n)
{
    // Napomena: skroz nepotrebna funkcija, služi samo za proveru tačnosti prilikom pisanja koda

    unsigned int error_count = 0;       // broj detektovanih grešaka 

    // iteracija kroz nizove kako bi utvrdili tačnost
    for (auto i = 0; i < n; i++)
    {
        if (A[i] + B[i] != C[i] && (i+1) % 256 != 0)
        {
            error_count++;
        }
    }

    // ispis rezultata u konzoli
    std::cout << "Total error count: " << error_count << std::endl;

    std::cout << "The first 10 numbers from each array: " << std::endl;
    for (int i = 0; i < 10; i++)
    {
        std::cout << i << " : " << A[i] << " " << B[i] << " " << C[i] << "\n";
    }

    std::cout << "Element 256 = " << C[255] << "\n";
    std::cout << "Element 512 = " << C[511] << "\n";
    std::cout << "Element 768 = " << C[767] << "\n";
}

int januar2023_zadatak1()
{
    const int n = 1000;                 // broj elemenata u svakom nizu
    float A_h[n]{};                     // niz A u glavnoj memoriji procesora ("hosta")
    float B_h[n]{};                     // niz B u glavnoj memoriji procesora ("hosta")
    float C_h[n]{};                     // niz C u glavnoj memoriji procesora ("hosta")

    // inicijalizacija nizova 
    for (auto i = 0; i < n; A_h[i++] = (float)(rand() % 10));
    for (auto i = 0; i < n; B_h[i++] = i);

    // poziv wrapper funkcije koja priprema i poziva kernel funkciju
    vecAdd(A_h, B_h, C_h, n);

    // dodatna metoda za proveru tačnosti - nije potrebna
    TestResults(A_h, B_h, C_h, n);

    return 0;
}