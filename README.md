# Parallel_Computing, NTU Singapore (C++, OpenMP, MPI, CUDA)

The shortest path problem is about finding a path between two nodes in a graph such that the path cost is minimized. One example of this problem could be finding the fastest route from one city to another by car, train or airplane.
The Floyd-Warshall algorithm is an algorithm that solves this problem. It works for weighted graphs with positive or negative weights but not for graphs with negative cycles.

It works by comparing all possible paths between all vertex pairs in the graph. A version of the algorithm implemented in the C language can be seen in the figure below.

for (int k = 0; k < N; k++) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) 
        {
            int i0 = i*N + j;
            int i1 = i*N + k;
            int i2 = k*N + j;
            if (mat[i1] != -1 && mat[i2] != -1)
            {
                int sum = (mat[i1] + mat[i2]);
                if (mat[i0] == -1 && sum < mat[i0])
                    mat[i0] = sum;
            }
        }
    }
}

The notations used in this figure; "k", "i", "j", "N" and "mat[...]" will be used throughout the report.

For each k all of the current values (mat[i*N,j]) of the matrix is compared to the sum of two other values in the matrix: mat[k*N,j] + mat[i*N,k]. Once the outer loop has run N times all paths between all vertex pairs have been compared.

The purpose of the lab was to parallelize this algorithm with the use of MPI, OpenMP and CUDA.
