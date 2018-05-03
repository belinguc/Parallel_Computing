////////////////////////////////////////////////////////////////////////////
//
// Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
//
// Please refer to the NVIDIA end user license agreement (EULA) associated
// with this source code for terms and conditions that govern your use of
// this software. Any use, reproduction, disclosure, or distribution of
// this software and related documentation outside the terms of the EULA
// is strictly prohibited.
//
////////////////////////////////////////////////////////////////////////////

/* Template project which demonstrates the basics on how to setup a project
* example application.
* Host code.
*/

// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// includes CUDA
#include <cuda_runtime.h>

// includes, project
#include <helper_cuda.h>
#include <helper_functions.h> // helper functions for SDK examples
#include "MatUtil.h"

////////////////////////////////////////////////////////////////////////////////
// declaration, forward
void runTest(int argc, char **argv);

extern "C"
void computeGold(float *reference, float *idata, const unsigned int len);
void GenMatrix(int *mat, const size_t N);
bool CmpArray(const int *l, const int *r, const size_t eleNum);
void ST_APSP(int *mat, const size_t N);
void printArray(const int *l, const size_t eleNum);

////////////////////////////////////////////////////////////////////////////////
//! APSP basic kernel for device functionality
//! @param g_idata  input data in global memory		Matrix
//! @param k 		input data in global memory		Current k
//! @param N  		input data in global memory		Size of the matrix
////////////////////////////////////////////////////////////////////////////////
__global__ void
apspKernel(int *g_idata, int k, int N)
{
	int mX = blockIdx.x*blockDim.x + threadIdx.x;
	int mY = blockIdx.y*blockDim.y + threadIdx.y;

	int i0 = mX*N + mY;
	int i1 = mX*N + k;
	int i2 = k*N + mY;
		
	if(g_idata[i1] != -1 && g_idata[i2] != -1)
     	{ 
  		int sum =  (g_idata[i1] + g_idata[i2]);
         	if (g_idata[i0] == -1 || sum < g_idata[i0])
		     g_idata[i0] = sum;
	}

}

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int
main(int argc, char **argv)
{
    runTest(argc, argv);
}

////////////////////////////////////////////////////////////////////////////////
//! Run a simple test for CUDA
////////////////////////////////////////////////////////////////////////////////
void
runTest(int argc, char **argv)
{
	size_t N = atoi(argv[1]);
	unsigned int mem_size = sizeof(int)*N*N;
	int *mat = (int*)malloc(mem_size);
	GenMatrix(mat, N);

	////////////////////////////////////////////////////////////////////////////////
	//! Compute the reference result
	////////////////////////////////////////////////////////////////////////////////
	StopWatchInterface *timerRef = 0;
	sdkCreateTimer(&timerRef);
	
	int *ref = (int*)malloc(mem_size);
	memcpy(ref, mat, mem_size);

	sdkStartTimer(&timerRef);
	ST_APSP(ref, N);
	sdkStopTimer(&timerRef);

	double tseq = sdkGetTimerValue(&timerRef);
	printf("Processing ref time: %f (ms)\n", tseq);
	// printf("Processing ref time: %f (ms)\n", sdkGetTimerValue(&timerRef));
	sdkDeleteTimer(&timerRef);


	////////////////////////////////////////////////////////////////////////////////
	//! Compute the parallel result withe the APSP basic kernel
	////////////////////////////////////////////////////////////////////////////////
	// use command-line specified CUDA device, otherwise use device with highest Gflops/s
	int devID = findCudaDevice(argc, (const char **)argv);

	StopWatchInterface *timer = 0;
	sdkCreateTimer(&timer);
	sdkStartTimer(&timer);	
	
	// allocate device memory
	int *d_idata;
	checkCudaErrors(cudaMalloc((void **) &d_idata, sizeof(int)*N*N));

	// copy host memory to device
	checkCudaErrors(cudaMemcpy(d_idata, mat, sizeof(int)*N*N,cudaMemcpyHostToDevice));
	
	// setup execution parameters
	int width = N;
	int tileWidth = 8; // 8x8 = 64 threads/block
	int sizeGrid = ceil(width/tileWidth);
	dim3  dimGrid(sizeGrid, sizeGrid, 1);
	dim3  dimBlock(tileWidth, tileWidth, 1);

	// execute the kernel
	for(int k = 0; k < N; k++){	
		apspKernel<<< dimGrid, dimBlock >>>(d_idata, k, N);
	}

	// check if kernel execution generated and error
	getLastCudaError("Kernel execution failed");

	// allocate mem for the result on host side
	int *result = (int *) malloc(mem_size);

	// copy result from device to host
	checkCudaErrors(cudaMemcpyAsync(result, d_idata, sizeof(int) * N*N, cudaMemcpyDeviceToHost, 0));

	sdkStopTimer(&timer);
	double tp = sdkGetTimerValue(&timer);
	printf("Processing parallel time: %f (ms)\n", tp);
	sdkDeleteTimer(&timer);


	////////////////////////////////////////////////////////////////////////////////
	//! Compute speedup and compare results
	////////////////////////////////////////////////////////////////////////////////
	double speed = tseq/tp;
	printf("Speed = %f \n", speed);

	//compare results
	if(CmpArray(result, ref, N*N))
	{
		printf("Your result is correct.\n");
		bool bTestResult = true;
	}
	else
	{
		printf("Your result is wrong.\n");
		bool bTestResult = false;
	}

	// cleanup memory
	free(ref);
	checkCudaErrors(cudaFree(d_idata));

	cudaDeviceReset();
	exit(bTestResult ? EXIT_SUCCESS : EXIT_FAILURE);
}
