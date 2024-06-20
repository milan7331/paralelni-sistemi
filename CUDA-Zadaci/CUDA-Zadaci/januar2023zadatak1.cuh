#pragma once

#include "cuda_common.h"

__global__ void vecAddKernel(float* A_d, float* B_d, float* C_d, int n);

__global__ void vecAddKernelModified(float* A_d, float* B_d, float* C_d, int n);

static int vecAdd(float* A, float* B, float* C, int n);

static void TestResults(float* A, float* B, float* C, int n);

int januar2023zadatak1();