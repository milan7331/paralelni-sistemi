#pragma once

#include "cuda_common.h"
#include <vector>

__global__ void MinimalElementKernel(float* device_matrix, float* device_min_element, int* device_array_size);

static int MinimalElement(float* A_host, float* min_element_host);

static int FillMatrix(float* A);

static bool TestResult(float* A, float* min_element_returned);

int jun2023_zadatak2();