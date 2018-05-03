#include "MatUtil.h"
#include <omp.h>

void GenMatrix(int *mat, const size_t N)
{
	for(int i = 0; i < N*N; i ++)
		mat[i] = rand()%32 - 1;
	for(int i = 0; i < N; i++)
		mat[i*N + i] = 0;

}

bool CmpArray(const int *l, const int *r, const size_t eleNum)
{
	for(int i = 0; i < eleNum; i ++)
		if(l[i] != r[i])
		{
			printf("ERROR: l[%d] = %d, r[%d] = %d\n", i, l[i], i, r[i]);
			return false;
		}
	return true;
}


/*
	Sequential (Single Thread) APSP on CPU.
*/
void ST_APSP(int *mat, const size_t N)
{
	for(int k = 0; k < N; k ++)
		for(int i = 0; i < N; i ++)
			for(int j = 0; j < N; j ++)
			{
				int i0 = i*N + j;
				int i1 = i*N + k;
				int i2 = k*N + j;
				if(mat[i1] != -1 && mat[i2] != -1)
                     { 
			          int sum =  (mat[i1] + mat[i2]);
                          if (mat[i0] == -1 || sum < mat[i0])
 					     mat[i0] = sum;
				}
			}
}

/*
	Parallel (Multiple Threads) APSP on CPU. 
*/
void MT_APSP(int *mat, const size_t N)
{
	// Set the number of threads + assigning iterations of parallel for to threads
	omp_set_dynamic(0);
	omp_set_num_threads(4);
	
	// Creating variables
	int i, j, k ; 

	// Iteration of k in sequential
	for(k = 0; k < N; k ++)
	{
		// Static assignment : Divide the matrix into groups of rows
		// and assign each group to a separate thread.
		// Each thread has its own variable i and j and share the matrix (mat)
		#pragma omp parallel for private(i,j) schedule (static)
		for(i = 0; i < N; i ++)
		{
			// Calculation of i in the first loop, independent of j
			int i1 = i*N + k;
			// Compute rows of the matrix with the row k
			for(j = 0; j < N; j ++)
			{
				int i0 = i*N + j;
				int i2 = k*N + j;
				if(mat[i1] != -1 && mat[i2] != -1)
		 		{ 
					int sum =  (mat[i1] + mat[i2]);
				  	if (mat[i0] == -1 || sum < mat[i0])
	 					mat[i0] = sum;
				}
			}
		}
	}
}


