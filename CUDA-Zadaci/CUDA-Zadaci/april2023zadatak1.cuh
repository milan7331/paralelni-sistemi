#pragma once

#include "cuda_common.h"

__global__ void AntiAliasingKernel(float* pic_device_in, float* pic_device_out, int depth);

static int AntiAliasing(float* pic_host_in, float* pic_host_out, int k);

static int FillPictures(float* pic_a, float* pic_b);

static int PrintOutSample(float* pic, int x_start, int y_start, int x_size, int y_size);

int april2023zadatak1();