#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include "MatUtil.h"

//gcc -o openmptest -fopenmp file1.c file1.h file2.c ... -std=c99

int main(int argc, char **argv)
{
	struct timeval tv1,tv2;	
	
	if(argc != 2)
	{
		printf("Usage: test {N}\n");
		exit(-1);
	}

	//generate a random matrix.
	size_t N = atoi(argv[1]);
	int *mat = (int*)malloc(sizeof(int)*N*N);
	GenMatrix(mat, N);

	//compute the reference result.
	int *ref = (int*)malloc(sizeof(int)*N*N);
	memcpy(ref, mat, sizeof(int)*N*N);

	gettimeofday(&tv1, NULL);
	ST_APSP(ref, N);
	gettimeofday(&tv2, NULL);
	double tseq=(tv2.tv_sec-tv1.tv_sec)*1000000+tv2.tv_usec-tv1.tv_usec;
	printf("Elasped time = %f usecs\n", tseq); 

	//compute your results
	int *result = (int*)malloc(sizeof(int)*N*N);
	memcpy(result, mat, sizeof(int)*N*N);
	gettimeofday(&tv1, NULL);
	//parallel algorithm
	MT_APSP(result, N);
	gettimeofday(&tv2, NULL);
	double tp=(tv2.tv_sec-tv1.tv_sec)*1000000+tv2.tv_usec-tv1.tv_usec;
	printf("Elasped time parallel = %f usecs\n", tp); 

	//compare your result with reference result
	if(CmpArray(result, ref, N*N))
		printf("Your result is correct.\n");
	else
		printf("Your result is wrong.\n");

	double speed = tseq/tp;
	printf("Speed = %f \n", speed);
}
