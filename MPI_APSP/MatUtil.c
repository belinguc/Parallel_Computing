#include "MatUtil.h"

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
	int rank;
	
	const int root = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if(rank == root)
	{
		//printMat(mat, n);
	}
			
	int *tmp_buf;
	tmp_buf = malloc(N*N*sizeof(int));
	
	
	for (int k = 0; k < N; k++)
	{
		if(rank == root)
		{
			//tmp_k = k;
			for(int i = 0; i < N*N; i++)
				tmp_buf[i] = mat[i];
		}
		

		//printf("Rank %d broadcasting current matrix...\n", rank);
		MPI_Bcast (tmp_buf, N*N, MPI_INT, root, MPI_COMM_WORLD);
		
		
		//printf("Rank %d calculating current matrix...\n", rank);
		functionName(tmp_buf, N, k );

		//printf("Rank %d collecting data with reduce...\n", rank);
		MPI_Reduce(tmp_buf, mat, N*N, MPI_INT, MPI_MIN, root, MPI_COMM_WORLD);
		//if(rank == 0)
			//printMat(mat, n);
	    
	}
	
}

/*
Parallel (Multiple Threads) APSP on CPU.
distributing parts of the matrix to different threads
*/

void MT_SM_APSP(int *mat, const size_t N)
{

	int rank;
	int numprocs;
	int owner;
	const int root = 0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	int lb = (N / numprocs)*rank;
	int ub = (N / numprocs)*(rank + 1);
	int row_data_size = (ub - lb)*N;
	int nbline = row_data_size / N;
	//printf("Rank : %d row_data_size = %d N= %d numprocs= %d nbline %d \n", rank, row_data_size, N, numprocs, nbline);

	//if (rank == root)
		//printMat(mat, n);

	int *tmp_row_buf;
	tmp_row_buf = malloc(row_data_size*sizeof(int));
	int *tmp_k_buf;
	tmp_k_buf = malloc(N*sizeof(int));

	//each processor has their own row
	MPI_Scatter(mat, row_data_size, MPI_INT, tmp_row_buf, row_data_size, MPI_INT, root, MPI_COMM_WORLD);
	//printMat2(tmp_row_buf, row_data_size, nbline);
	//printBuf(tmp_row_buf, row_data_size);


	for (int k = 0; k < N; k++)
	{
		//with gather
		if (rank == root)
		{
			for (int i = 0; i < N; i++)
				tmp_k_buf[i] = mat[k*N + i];
			//printf("tmp_k_buf ...\n");
			//printBuf(tmp_k_buf, N);
		}

		//without gather
		/*if (k > lb && k < ub)
		{
			printf("Rank = %d tmp_k_buf ...\n", rank);
			for (int i = 0; i < N; i++)
				tmp_k_buf[i] = mat[k*N + i];
			//printf("tmp_k_buf ...\n");
			//printBuf(tmp_k_buf, N);
		}*/

		//printf("Rank %d broadcasting current row k = %d ...\n", rank, k);
		MPI_Bcast(tmp_k_buf, N, MPI_INT, root, MPI_COMM_WORLD);

		//printf("Rank %d calculating current matrix...\n", rank);
		functionName2(tmp_row_buf, tmp_k_buf, N, k, row_data_size);
		
		//printf("Rank %d gather...\n", rank);
		//update row k by the owner simplifier apres juste update pas refaire la matrice 
		MPI_Gather(tmp_row_buf, row_data_size, MPI_INT, mat, row_data_size, MPI_INT, root, MPI_COMM_WORLD);
		
		/*if (rank == 0)
		{
			printMat(mat, N);
			exit(-1);
		}*/

	}

	//MPI_Gather(tmp_row_buf, row_data_size, MPI_INT, mat, row_data_size, MPI_INT, root, MPI_COMM_WORLD);

}

void functionName(int *mat, const int N, const int K)
{
	
	int rank, numprocs;
	
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	int lb = (N / numprocs)*rank;
	int ub = (N / numprocs)*(rank+1);
	//printf("rank = %d K = %d N = %d lb = %d ub = %d \n", rank, K, N, lb, ub);
	
	for (int i = lb; i < ub; i++)
	{
		for (int j = 0; j < N; j++)
		{
			int i0 = i*N + j;
			int i1 = i*N + K;
			int i2 = K*N + j;
			if (mat[i1] != -1 && mat[i2] != -1)
			{
				int sum = (mat[i1] + mat[i2]);
				if (mat[i0] == -1 || sum < mat[i0])
					mat[i0] = sum;
			}
		}
	}
	
}

void functionName2(int *mat, int *linek, const int N, const int K, const int row_data_size)
{
	
	int rank, numprocs;
	int nbline = row_data_size / N;
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	//printf("rank = %d in functionNqme2, print mat \n", rank);
	//printBuf(mat, row_data_size);

	//printf("rank = %d in functionNqme2, print linek \n", rank);
	//printBuf(linek, N);

	for (int i = 0; i < nbline; i++)
	{
		for (int j = 0; j < N; j++)
		{
			int i0 = i*N + j;
			int i1 = i*N + K;
			//int i2 = K*N + j;
			//int i2 = j;
			if (mat[i1] != -1 && linek[j] != -1)
			{
				int sum = (mat[i1] + linek[j]);
				if (mat[i0] == -1 || sum < mat[i0])
					mat[i0] = sum;
			}
		}
	}

}

void printMat(int *mat, const int N)
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	printf("salut rank = %d!\n", rank);
	for(int i = 0; i < N; i++)
	{
		for(int j = 0; j < N; j++)
		{
			printf(" %d", mat[i*N + j]);
		}
		printf("\n");
	}
	printf("Au revoir!\n");	
}

void printMat2(int *mat, const int N, const int line)
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	printf("salut rank = %d!\n", rank);
	for (int i = 0; i < line; i++)
	{
		for (int j = 0; j < N; j++)
		{
			printf(" %d", mat[i*N + j]);
		}
		printf("\n");
	}
	printf("Au revoir!\n");
}

void printBuf(int *mat, const int N)
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	printf("salut buf rank = %d!\n", rank);
	for (int i = 0; i < N; i++)
	{
		printf(" %d", mat[i]);
		printf("\n");
	}
	printf("Au revoir buf !\n");
}