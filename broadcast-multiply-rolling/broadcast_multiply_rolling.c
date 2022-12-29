#include <mpi.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct {
    int rows;
    int columns;
    int *data;
} MatrixBlock;

MatrixBlock* partition_and_distribute_matrix_blocks(
    const MPI_Comm *comm_grid,
    int matrix_rows,
    int matrix_columns,
    int *global_matrix
);

void create_square_grid_processes(
    MPI_Comm *comm_grid,
    unsigned int dim
);

MatrixBlock *distribute_main_diagonal_blocks(
    const MPI_Comm *comm_grid,
    MatrixBlock *local_matrix_A
);

MatrixBlock *broadcast_rolling(
    int step,
    const MPI_Comm *comm_grid,
    MatrixBlock *local_matrix_A,
    MatrixBlock *local_matrix_B
);

MatrixBlock *gather_partial_solutions(
    int root,
    MPI_Comm *comm_grid,
    MatrixBlock *partial_solution,
    int global_matrix_rows,
    int global_matrix_columns
);

MatrixBlock *multiply_matrices(MatrixBlock *A, MatrixBlock *B);
MatrixBlock *sum_matrices(MatrixBlock *A, MatrixBlock *B);

void print_matrix(int rows, int columns, int *matrix);
void print_vector(int *v, int size);

int get_square_matrix_dim_command_line(char **argv);
int get_comm_grid_dim_command_line(char **argv, unsigned int total_processes);

bool is_number(const char *s);

void check_arguments(int argc, char **argv);
void check_validity_square_cartesian_topology_processes(
    int square_matrix_dim,
    int total_processes,
    int square_cartesian_topology_dim,
    int pid_comm_world
);

void print_result(
    int square_matrix_dim,
    const MPI_Comm *comm_grid,
    int *global_matrix_A,
    int *global_matrix_B,
    MatrixBlock *local_result_matrix,
    MatrixBlock *total_result_matrix
);

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    check_arguments(argc, argv);
    const int square_matrix_dim = get_square_matrix_dim_command_line(argv);
    const int square_matrix_total_elements = square_matrix_dim * square_matrix_dim;

    int total_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &total_processes);

    if (square_matrix_total_elements < total_processes) {
        fprintf(stderr,
            "The number of total processes is greater than %d x %d!\n",
            square_matrix_dim,
            square_matrix_dim
        );
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    int pid_comm_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid_comm_world);

    int comm_grid_dim = (pid_comm_world == 0)
       ? get_comm_grid_dim_command_line(argv, total_processes)
       : 0;

    MPI_Bcast(&comm_grid_dim, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    check_validity_square_cartesian_topology_processes(
        square_matrix_dim,
        total_processes,
        comm_grid_dim,
        pid_comm_world
    );

    MPI_Comm *comm_grid = (MPI_Comm *) malloc(sizeof(MPI_Comm));
    create_square_grid_processes(
        comm_grid,
        comm_grid_dim
    );

    int pid_comm_grid;
    MPI_Comm_rank(*comm_grid, &pid_comm_grid);

    /* Create and fill the global matrix A and B */
    int global_matrix_A[square_matrix_total_elements];
    int global_matrix_B[square_matrix_total_elements];
    if (pid_comm_grid == 0) {
        for (int i = 0; i < square_matrix_total_elements; i++) {
            global_matrix_A[i] = i;
            global_matrix_B[i] = i;
        }
    }

    /* Get local matrix block A */
    MatrixBlock *local_matrix_block_A =
        partition_and_distribute_matrix_blocks(
            comm_grid,
            square_matrix_dim,
            square_matrix_dim,
            global_matrix_A
        );

    /* Get local matrix block B */
    MatrixBlock *local_matrix_block_B =
        partition_and_distribute_matrix_blocks(
            comm_grid,
            square_matrix_dim,
            square_matrix_dim,
            global_matrix_B
        );

    MPI_Barrier(*comm_grid);
    const double t0 = MPI_Wtime();
    

    // Start BROADCAST MULTIPLY ROLLING STEP 0
    // Each block of matrix A that is on the main diagonal is distributed to
    // all processes that are on the same row of the block.
    MatrixBlock *local_main_diagonal_matrix_block_A = distribute_main_diagonal_blocks(comm_grid, local_matrix_block_A);

    // Calculates a component of the local solution
    MatrixBlock *local_result_matrix = multiply_matrices(local_main_diagonal_matrix_block_A, local_matrix_block_B);

    // At each new step, the diagonal that is #step times upward from the main diagonal is considered.
    // Then the same operation as in step 0 is repeated.
    // In addition, each process passes its current block of B to
    // the processor located just above it in the process grid.
    MatrixBlock *current_matrix_B = local_matrix_block_B;
    for (int step = 1; step < comm_grid_dim; step++) {
        MatrixBlock *matrix_blocks = broadcast_rolling(
            step,
            comm_grid,
            local_matrix_block_A,
            current_matrix_B
        );
        current_matrix_B = &matrix_blocks[1];
        // At each step a new component of the local solution is calculated, and it is added to the total local solution
        local_result_matrix = sum_matrices(
            local_result_matrix,
            multiply_matrices(&matrix_blocks[0], current_matrix_B)
        );
    }

    const double t1 = MPI_Wtime();

    double total_time = t1 - t0;
    double max_total_time;
    MPI_Reduce(
        &total_time,
        &max_total_time,
        1,
        MPI_DOUBLE,
        MPI_MAX,
        0,
        *comm_grid
    );

    MatrixBlock *total_result_matrix = gather_partial_solutions(
        0,
        comm_grid,
        local_result_matrix,
        square_matrix_dim,
        square_matrix_dim
    );

    print_result(
        square_matrix_dim,
        comm_grid,
        global_matrix_A,
        global_matrix_B,
        local_result_matrix,
        total_result_matrix
    );

    if (pid_comm_grid == 0) {
        printf("[TOTAL TIME PARALLEL] %e seconds.\n", max_total_time);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}

void check_validity_square_cartesian_topology_processes(
    const int square_matrix_dim,
    const int total_processes,
    const int square_cartesian_topology_dim,
    const int pid_comm_world
) {
    if (total_processes % square_cartesian_topology_dim != 0) {
        if (pid_comm_world == 0) {
            fprintf(stderr,
                "The total number of processes must be divisible by the dimension "
                "of the square cartesian topology of processes (argument <grid_comm_dim>)!\n"
            );
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (square_matrix_dim % square_cartesian_topology_dim != 0) {
        if (pid_comm_world == 0) {
            fprintf(stderr,
                "The dimension of the square matrix must be divisible by the dimension "
                "of the square cartesian topology of processes (argument <grid_comm_dim>)!\n"
            );
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
}

MatrixBlock *multiply_matrices(MatrixBlock *A, MatrixBlock *B) {
    if (A->columns != B->rows) return NULL;
    int *result = (int*) malloc(sizeof(int) * A->rows * B->columns);
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < B->columns; j++) {
            result[i * B->columns + j] = 0;
            for (int k = 0; k < B->rows; k++) {
               result[i * B->columns + j] += A->data[i * A->columns + k] * B->data[k * B->columns + j];
            }
        }
    }
    MatrixBlock *C = (MatrixBlock*) malloc(sizeof(MatrixBlock));
    C->data = result;
    C->rows = A->rows;
    C->columns = B->columns;
    return C;
}

MatrixBlock *sum_matrices(MatrixBlock *A, MatrixBlock *B) {
    if (A->rows != B->rows || A->columns != B->columns) return NULL;
    const int rows = A->rows; const int columns = A->columns;
    int *result = (int*) malloc(sizeof(int) * A->rows * A->columns);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            result[i * columns + j] = A->data[i * columns + j] + B->data[i * columns + j];
        }
    }
    MatrixBlock *C = (MatrixBlock*) malloc(sizeof(MatrixBlock));
    C->data = result;
    C->rows = A->rows;
    C->columns = A->columns;
    return C;
}

void print_vector(int *v, int size) {
    for (int i = 0; i < size; i++)
        printf("%s%d%s", i == 0 ? "[" : "", v[i], i == size - 1 ? "]\n" : " ");
}

void print_matrix(int rows, int columns, int *matrix) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            printf("%3d ",(int)matrix[i * columns + j]);
        }
        printf("\n");
    }
    printf("\n");
}

void create_square_grid_processes(
    MPI_Comm *comm_grid,
    const unsigned int dim
) {
    static const int N_DIMS = 2;

    int *dims = (int*)malloc(sizeof(int) * N_DIMS);
    dims[0] = (int)dim;
    dims[1] = (int)dim;

    const int *periods = (int*)calloc(N_DIMS, sizeof(int));
    const int reorder = 0;

    MPI_Cart_create(MPI_COMM_WORLD, N_DIMS, dims, periods, reorder, comm_grid);
}

MatrixBlock* partition_and_distribute_matrix_blocks(
    const MPI_Comm *comm_grid,
    const int matrix_rows,
    const int matrix_columns,
    int *global_matrix
) {
    int pid_comm_grid;
    MPI_Comm_rank(*comm_grid, &pid_comm_grid);

    int dims[2], periods[2], coords[2];
    MPI_Cart_get(*comm_grid, 2, dims, periods, coords);
    const int comm_grid_total_rows = dims[0];
    const int comm_grid_total_columns = dims[1];
    const int total_processes = comm_grid_total_rows * comm_grid_total_columns;

    const int block_rows = matrix_rows / comm_grid_total_rows;
    const int block_columns = matrix_columns / comm_grid_total_columns;

    int *local_matrix = (int*)malloc(sizeof(int) * block_rows * block_columns);

    MPI_Datatype blocktype_not_resized;
    MPI_Datatype blocktype_resized;

    MPI_Type_vector(
        block_rows,
        block_columns,
        matrix_columns,
        MPI_INT,
        &blocktype_not_resized
    );
    MPI_Type_create_resized( blocktype_not_resized, 0, sizeof(int), &blocktype_resized);
    MPI_Type_commit(&blocktype_resized);

    int displs[total_processes];
    int send_counts[total_processes];
    for (int i = 0; i < comm_grid_total_rows; i++) {
        for (int j = 0; j < comm_grid_total_columns; j++) {
            displs[i * comm_grid_total_columns + j] = i * matrix_columns * block_rows + j * block_columns;
            send_counts[i * comm_grid_total_columns + j] = 1;
        }
    }

    MPI_Scatterv(
        global_matrix,
        send_counts,
        displs,
        blocktype_resized,
        local_matrix,
        block_rows * block_columns,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    MatrixBlock *matrix_block = (MatrixBlock*)malloc(sizeof(MatrixBlock));
    matrix_block->columns = block_columns;
    matrix_block->rows = block_rows;
    matrix_block->data = local_matrix;
    return matrix_block;
}

MatrixBlock *distribute_main_diagonal_blocks(
    const MPI_Comm *comm_grid,
    MatrixBlock *local_matrix_A
) {
    int dims[2], periods[2], coords[2];
    MPI_Cart_get(*comm_grid, 2, dims, periods, coords);

    int rank;
    MPI_Cart_rank(*comm_grid, coords, &rank);

    MPI_Comm *newcomm_rows = (MPI_Comm*) malloc(sizeof(MPI_Comm));
    const int remain_dims[2] = {false, true};
    MPI_Cart_sub(*comm_grid, remain_dims, newcomm_rows);
    int newcomm_rows_rank;
    MPI_Comm_rank(*newcomm_rows, &newcomm_rows_rank);

    int roots[dims[0] * dims[1]];
    int count = 0;
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            roots[count] = i;
            count++;
        }
    }

    const int size_matrix = (local_matrix_A->rows) * (local_matrix_A->columns);
    int *matrix = (roots[rank] == newcomm_rows_rank)
        ? local_matrix_A->data
        : (int*) calloc(sizeof(int), size_matrix);

    MPI_Bcast(
        matrix,
        size_matrix,
        MPI_INT,
        roots[rank],
        *newcomm_rows
    );

    MatrixBlock *matrix_block = (MatrixBlock*)malloc(sizeof(MatrixBlock));
    matrix_block->columns = local_matrix_A->columns;
    matrix_block->rows = local_matrix_A->rows;
    matrix_block->data = matrix;
    return matrix_block;
}

MatrixBlock *broadcast_rolling(
    const int step,
    const MPI_Comm *comm_grid,
    MatrixBlock *local_matrix_A,
    MatrixBlock *local_matrix_B
) {
    int dims[2], periods[2], coords[2];
    MPI_Cart_get(*comm_grid, 2, dims, periods, coords);

    int rank;
    MPI_Cart_rank(*comm_grid, coords, &rank);

    MPI_Comm *newcomm_rows = (MPI_Comm*) malloc(sizeof(MPI_Comm));
    const int remain_dims[2] = {false, true};
    MPI_Cart_sub(*comm_grid, remain_dims, newcomm_rows);
    int newcomm_rows_rank;
    MPI_Comm_rank(*newcomm_rows, &newcomm_rows_rank);

    int roots[dims[0] * dims[1]];
    int count = 0;
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            roots[count] = (i + step) % dims[0];
            count++;
        }
    }

    const int size_matrix = (local_matrix_A->rows) * (local_matrix_A->columns);
    int *matrix_A = (roots[rank] == newcomm_rows_rank)
      ? local_matrix_A->data
      : (int*) malloc(sizeof(int) * size_matrix);

    MPI_Bcast(
        matrix_A,
        size_matrix,
        MPI_INT,
        roots[rank],
        *newcomm_rows
    );

    MPI_Comm *newcomm_columns = (MPI_Comm*) malloc(sizeof(MPI_Comm));
    const int columns_remain_dims[2] = {true, false};
    MPI_Cart_sub(*comm_grid, columns_remain_dims, newcomm_columns);

    const int size_matrix_B = local_matrix_B->rows * local_matrix_B->columns;

    int newcomm_columns_rank;
    MPI_Comm_rank(*newcomm_columns, &newcomm_columns_rank);

    const static int TAG = 80;
    MPI_Request *mpi_request = (MPI_Request*) malloc(sizeof(MPI_Request));

    const int previous_process_rank = abs(newcomm_columns_rank - 1) % dims[0];

    MPI_Isend(
        local_matrix_B->data,
        size_matrix_B,
        MPI_INT,
        previous_process_rank,
        TAG + newcomm_columns_rank,
        *newcomm_columns,
        mpi_request
    );

    const int next_process_rank = (newcomm_columns_rank + 1) % dims[0];
    int *matrix_B = (int*) malloc(sizeof(int) * size_matrix_B);
    MPI_Recv(
        matrix_B,
        size_matrix_B,
        MPI_INT,
        next_process_rank,
        TAG + next_process_rank,
        *newcomm_columns,
        MPI_STATUSES_IGNORE
    );

    MPI_Request_free(mpi_request);

    MatrixBlock *matrix_blocks = (MatrixBlock*) malloc(sizeof(MatrixBlock) * 2);
    matrix_blocks[0].data = matrix_A;
    matrix_blocks[0].rows = local_matrix_A->rows;
    matrix_blocks[0].columns = local_matrix_A->columns;
    matrix_blocks[1].data = matrix_B;
    matrix_blocks[1].rows = local_matrix_B->rows;
    matrix_blocks[1].columns = local_matrix_B->columns;
    return matrix_blocks;
}

MatrixBlock *gather_partial_solutions(
    const int root,
    MPI_Comm *comm_grid,
    MatrixBlock *partial_solution,
    const int global_matrix_rows,
    const int global_matrix_columns
) {
    int pid_comm_grid;
    MPI_Comm_rank(*comm_grid, &pid_comm_grid);

    int dims[2], periods[2], coords[2];
    MPI_Cart_get(*comm_grid, 2, dims, periods, coords);
    const int comm_grid_total_rows = dims[0];
    const int comm_grid_total_columns = dims[1];
    const int total_processes = comm_grid_total_rows * comm_grid_total_columns;

    int displs[total_processes];
    int recv_counts[total_processes];
    for (int i = 0; i < comm_grid_total_rows; i++) {
        for (int j = 0; j < comm_grid_total_columns; j++) {
            displs[i * comm_grid_total_columns + j] =
                    i * global_matrix_columns * partial_solution->rows +
                    j * partial_solution->columns;
            recv_counts[i * comm_grid_total_columns + j] = 1;
        }
    }

    MPI_Datatype blocktype_not_resized;
    MPI_Datatype blocktype_resized;
    MPI_Type_vector(
        partial_solution->rows,
    partial_solution->columns,
        global_matrix_columns,
        MPI_INT,
        &blocktype_not_resized
    );
    MPI_Type_create_resized( blocktype_not_resized, 0, sizeof(int), &blocktype_resized);
    MPI_Type_commit(&blocktype_resized);

    int *total_solution_data = NULL;
    if (pid_comm_grid == root) {
        total_solution_data = (int*) malloc(sizeof(int) * global_matrix_columns * global_matrix_rows);
    }

    MPI_Gatherv(
        partial_solution->data,
        partial_solution->rows * partial_solution->columns,
        MPI_INT,
        total_solution_data,
        recv_counts,
        displs,
        blocktype_resized,
        root,
        MPI_COMM_WORLD
    );

    MatrixBlock *total_solution = NULL;
    if (pid_comm_grid == root) {
        total_solution = (MatrixBlock*) malloc(sizeof(MatrixBlock));
        total_solution->rows = global_matrix_rows;
        total_solution->columns = global_matrix_columns;
        total_solution->data = total_solution_data;
    }
    return total_solution;
}

void check_arguments(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Correct usage: %s <grid_comm_dim> <square_matrix_dim>\n", argv[0]);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
}

int get_square_matrix_dim_command_line(char **argv) {
    const static int SQUARE_MATRIX_DIM_ARGV_INDEX = 2;
    const static char *SQUARE_MATRIX_DIM_ARGV_NAME = "<square_matrix_dim>";
    if (!is_number(argv[SQUARE_MATRIX_DIM_ARGV_INDEX])) {
        fprintf(stderr, "%s must be a positive number!\n", SQUARE_MATRIX_DIM_ARGV_NAME);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    const int dim = atoi(argv[SQUARE_MATRIX_DIM_ARGV_INDEX]);
    if (dim < 1) {
        fprintf(stderr, "%s must be greater than one!\n", SQUARE_MATRIX_DIM_ARGV_NAME);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    return dim;
}

int get_comm_grid_dim_command_line(char **argv, const unsigned int total_processes) {
    const static int GRID_COMM_DIM_ARGV_INDEX = 1;
    const static char *GRID_COMM_DIM_ARGV_NAME = "<grid_comm_dim>";
    if (!is_number(argv[GRID_COMM_DIM_ARGV_INDEX])) {
        fprintf(stderr, "%s must be a positive number!\n", GRID_COMM_DIM_ARGV_NAME);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    int grid_comm_dim = atoi(argv[GRID_COMM_DIM_ARGV_INDEX]);
    if (grid_comm_dim < 1) {
        fprintf(stderr, "%s must be greater than one!\n", GRID_COMM_DIM_ARGV_NAME);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    if (grid_comm_dim > total_processes) {
        fprintf(
            stderr,
            "%s can't be greater than the number of processes %d!\n",
            GRID_COMM_DIM_ARGV_NAME, total_processes
        );
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    return grid_comm_dim;
}

bool is_number(const char *s) {
    if (s == NULL) return false;
    for (int i = 0; s[i] != '\0'; i++) {
        if (i == 0 && s[i] == '-' && isdigit(s[i + 1])) continue;
        if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

void print_result(
    const int square_matrix_dim,
    const MPI_Comm *comm_grid,
    int *global_matrix_A,
    int *global_matrix_B,
    MatrixBlock *local_result_matrix,
    MatrixBlock *total_result_matrix
) {
    int dims[2], periods[2], coords[2];
    MPI_Cart_get(*comm_grid, 2, dims, periods, coords);
    const int comm_grid_dim = dims[0];

    int pid_comm_grid;
    MPI_Comm_rank(*comm_grid, &pid_comm_grid);

    const char *PRINT_MODE_ENV_VAR = getenv("PRINT_MODE");
    const bool is_printing_mode_active = PRINT_MODE_ENV_VAR && strcmp(PRINT_MODE_ENV_VAR, "true") == 0;
    if (is_printing_mode_active) {
        if (pid_comm_grid == 0) {
            printf("\nMatrix A: \n");
            print_matrix(square_matrix_dim, square_matrix_dim, global_matrix_A);
            printf("Matrix B: \n");
            print_matrix(square_matrix_dim, square_matrix_dim, global_matrix_B);
            printf("Matrix C: \n");
            print_matrix(total_result_matrix->rows, total_result_matrix->columns, total_result_matrix->data);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        printf("[PID %d Coords (%d, %d)] Local result: \n", pid_comm_grid, coords[0], coords[1]);
        print_matrix(local_result_matrix->rows, local_result_matrix->columns, local_result_matrix->data);
        MPI_Barrier(MPI_COMM_WORLD);
    }
}