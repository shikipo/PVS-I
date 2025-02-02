// mpiexec -n 4 ./transpose.exe 4

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void print_matrix(double *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%6.1f ", matrix[i * n + j]);
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int rank, size, n;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
/*     if (argc != 2) {
        if (rank == 0) {
            printf("Usage: %s <matrix_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    } */

    n = atoi(argv[1]); // Matrix size (nxn)
/*     if (n % size != 0) {
        if (rank == 0) {
            printf("Matrix size should be divisible by the number of processes.\n");
        }
        MPI_Finalize();
        return 1;
    } */

    int rows_per_proc = n / size;
    double *local_matrix = (double *)malloc(rows_per_proc * n * sizeof(double)); //2D matrix stored as a 1D array
    double *global_matrix = NULL;

    if (rank == 0) {
        global_matrix = (double *)malloc(n * n * sizeof(double));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                global_matrix[i * n + j] = (double)(i * n + j);
        
        printf("Original Matrix:\n");
        print_matrix(global_matrix, n);
    }

    // Scatter the matrix rows to processes
    MPI_Scatter(global_matrix, rows_per_proc * n, MPI_DOUBLE, local_matrix, rows_per_proc * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Allocate space for transposed local data
    double *local_transposed = (double *)malloc(rows_per_proc * n * sizeof(double));

    // Correct transposition of local data
    for (int i = 0; i < rows_per_proc; i++)
        for (int j = 0; j < n; j++)
            local_transposed[j * rows_per_proc + i] = local_matrix[i * n + j];

    // Gather transposed data to root
    double *transposed_matrix = NULL;
    if (rank == 0) transposed_matrix = (double *)malloc(n * n * sizeof(double));

    MPI_Gather(local_transposed, rows_per_proc * n, MPI_DOUBLE, transposed_matrix, rows_per_proc * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Final rearrangement (fix the incorrect structure)
    if (rank == 0) {
        double *final_matrix = (double *)malloc(n * n * sizeof(double));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                final_matrix[i * n + j] = transposed_matrix[j * n + i]; // Fix row/col placement

        printf("\nfinal_matrix:\n");
        print_matrix(final_matrix, n);
        free(global_matrix);
        free(transposed_matrix);
        free(final_matrix);
    }

    free(local_matrix);
    free(local_transposed);
    MPI_Finalize();
    return 0;
}
