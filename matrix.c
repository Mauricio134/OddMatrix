#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void matrix_vector_multiply(const int* row, const int* vector, int* result, int cols) {
    *result = 0;  
    for (int j = 0; j < cols; ++j) {
        *result += row[j] * vector[j];
    }
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int rows = 4;   
    const int cols = 4;  

    int* matrix = (int*)malloc(rows * cols * sizeof(int));
    int* vector = (int*)malloc(cols * sizeof(int));
    int* local_matrix = (int*)malloc((rows / size) * cols * sizeof(int));  
    int* local_result = (int*)malloc((rows / size) * sizeof(int));         

    if (rank == 0) {
        // Use static size arrays to initialize
        int temp_matrix[4 * 4] = {
            1, 2, 3, 4,
            5, 6, 7, 8,
            9, 10, 11, 12,
            13, 14, 15, 16
        };
        for (int i = 0; i < rows * cols; i++) {
            matrix[i] = temp_matrix[i];
        }

        int temp_vector[4] = {1, 1, 1, 1};      
        for (int i = 0; i < cols; i++) {
            vector[i] = temp_vector[i];
        }
    }

    MPI_Bcast(vector, cols, MPI_INT, 0, MPI_COMM_WORLD);

    int base_rows_per_process = rows / size;
    int remaining_rows = rows % size; 

    MPI_Scatter(matrix, base_rows_per_process * cols, MPI_INT, local_matrix, base_rows_per_process * cols, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < base_rows_per_process; ++i) {
        matrix_vector_multiply(local_matrix + i * cols, vector, &local_result[i], cols);
    }

    int* result = (int*)malloc(rows * sizeof(int));
    MPI_Gather(local_result, base_rows_per_process, MPI_INT, result, base_rows_per_process, MPI_INT, 0, MPI_COMM_WORLD);

    if (remaining_rows != 0) {
        if (rank == 0) {
            for (int i = 0; i < remaining_rows; ++i) {
                int row_to_process = base_rows_per_process * size + i;
                MPI_Send(matrix + row_to_process * cols, cols, MPI_INT, i + 1, 0, MPI_COMM_WORLD);
            }
            for (int i = 0; i < remaining_rows; ++i) {
                int row_to_process = size + i;
                int additional_result = 0;
                MPI_Recv(&additional_result, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                result[row_to_process] = additional_result;
            }
        } else {
            if (rank <= remaining_rows) {
                int* additional_row = (int*)malloc(cols * sizeof(int));
                int recv_signal;

                MPI_Recv(additional_row, cols, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int additional_result = 0;
                matrix_vector_multiply(additional_row, vector, &additional_result, cols);
                MPI_Send(&additional_result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                free(additional_row);
            }
        }
    }

    if (rank == 0) {
        printf("Resultado (Matriz * Vector):\n");
        for (int i = 0; i < rows; ++i) {
            printf("%d\n", result[i]);
        }
    }

    free(matrix);
    free(vector);
    free(local_matrix);
    free(local_result);
    free(result);
    
    MPI_Finalize();
    return 0;
}
