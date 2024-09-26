#include <mpi.h>
#include <iostream>
#include <vector>

void matrix_vector_multiply(const int* row, const std::vector<int>& vector, int& result, int cols) {
    result = 0;  
    for (int j = 0; j < cols; ++j) {
        result += row[j] * vector[j];
    }
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int rows = 4;   
    const int cols = 4;  

    std::vector<int> matrix(rows * cols);
    std::vector<int> vector(cols);
    std::vector<int> local_matrix(rows / size * cols);  
    std::vector<int> local_result(rows / size);         

    if (rank == 0) {
        matrix = {1, 2, 3, 4,
                  5, 6, 7, 8,
                  9, 10, 11, 12,
                  13, 14, 15, 16};  

        vector = {1, 1, 1, 1};      
    }


    MPI_Bcast(vector.data(), cols, MPI_INT, 0, MPI_COMM_WORLD);

    int base_rows_per_process = rows / size;
    int remaining_rows = rows % size; 

 
    MPI_Scatter(matrix.data(), base_rows_per_process * cols, MPI_INT, local_matrix.data(), base_rows_per_process * cols, MPI_INT, 0, MPI_COMM_WORLD);


    for (int i = 0; i < base_rows_per_process; ++i) {
        matrix_vector_multiply(local_matrix.data() + i * cols, vector, local_result[i], cols);
    }


    std::vector<int> result(rows);
    MPI_Gather(local_result.data(), base_rows_per_process, MPI_INT, result.data(), base_rows_per_process, MPI_INT, 0, MPI_COMM_WORLD);

    if(remaining_rows!=0){
        if (rank == 0) {

            for (int i = 0; i < remaining_rows; ++i) {
                //std::cout<<"remining: "<<remaining_rows<<std::endl;
                int row_to_process = base_rows_per_process * size + i;
                //std::cout<<"row to process: "<<row_to_process<<std::endl;
                MPI_Send(matrix.data() + row_to_process *cols , cols, MPI_INT, i+1 , 0, MPI_COMM_WORLD);
            }
            for (int i = 0; i < remaining_rows; ++i) {
                int row_to_process = size + i;
                int additional_result = 0;
                MPI_Recv(&additional_result, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                result[row_to_process] = additional_result;
            }

            
        } else {
            if (rank <= remaining_rows){
                    std::vector<int> additional_row(cols);
                    int recv_signal;

                    MPI_Recv(additional_row.data(), cols, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    //std::cout<<"recibido "<<std::endl;
                    int additional_result = 0;
                    matrix_vector_multiply(additional_row.data(), vector, additional_result, cols);
                    //std::cout<<"calculated "<<additional_result<<std::endl;

                    MPI_Send(&additional_result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            }
        }
    }
    if (rank == 0) {
        std::cout << "Resultado (Matriz * Vector):\n";
        for (int i = 0; i < rows; ++i) {
            std::cout << result[i] << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}
