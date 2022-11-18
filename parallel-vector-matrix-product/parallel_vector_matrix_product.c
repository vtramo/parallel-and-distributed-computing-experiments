#include <mpi.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
 *  Given P processes, a matrix A of dimensions M x N, a vector x of dimension N and
 *  the total number of rows p of the cartesian topology of processes, the algorithm
 *  creates a bidimensional grid of processes of size p x (P/p), partitions, distributes the matrix
 *  to these processes and computes the product A * x = b in parallel.
 *
 *  Preconditions:
 *  - M * N >= P
 *  - P % p == 0
 *  - M % p == 0
 *  - N % (P/p) == 0
 */

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

void create_bidimensional_grid(
    MPI_Comm *comm_grid,
    unsigned int rows,
    unsigned int columns
);

int *calculate_partial_solution(MatrixBlock *matrix_block, const int *x, int x_size, const int* comm_grid_coords);

void print_matrix(int rows, int columns, const int *matrix);
void print_vector(int *v, int size);

int get_matrix_columns_command_line(char **argv);
int get_matrix_rows_command_line(char **argv);
int get_number_of_comm_grid_rows_command_line(char **argv, unsigned int total_processes);

bool is_number(const char *s);

void check_arguments(int argc, char **argv);
void check_validity_cartesian_topology_processes(
    int total_rows_matrix,
    int total_columns_matrix,
    int total_processes,
    int comm_grid_total_rows,
    int comm_grid_total_columns,
    int pid_comm_world
);

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    check_arguments(argc, argv);
    const int COLS = get_matrix_columns_command_line(argv);
    const int ROWS = get_matrix_rows_command_line(argv);

    int total_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &total_processes);

    if (ROWS * COLS < total_processes) {
        fprintf(stderr, "The number of total processes is greater than %d x %d!\n", ROWS, COLS);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    int pid_comm_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid_comm_world);

    int comm_grid_total_rows = (pid_comm_world == 0)
        ? get_number_of_comm_grid_rows_command_line(argv, total_processes)
        : 0;

    MPI_Bcast(&comm_grid_total_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);

    const int comm_grid_total_columns = total_processes / comm_grid_total_rows;

    check_validity_cartesian_topology_processes(
        ROWS,
        COLS,
        total_processes,
        comm_grid_total_rows,
        comm_grid_total_columns,
        pid_comm_world
    );

    MPI_Comm *comm_grid = (MPI_Comm*)malloc(sizeof(MPI_Comm));
    create_bidimensional_grid(
        comm_grid,
        comm_grid_total_rows,
        comm_grid_total_columns
    );

    int pid_comm_grid;
    MPI_Comm_rank(*comm_grid, &pid_comm_grid);

    /* Create and fill the global matrix */
    int global_matrix[ROWS * COLS];
    if (pid_comm_grid == 0) {
        for (int i = 0; i < ROWS * COLS; i++) {
            global_matrix[i] = i;
        }
    }

    /* Get matrix block */
    MatrixBlock *local_matrix_block =
        partition_and_distribute_matrix_blocks(
            comm_grid,
            ROWS,
            COLS,
            global_matrix
        );

    /* Create and fill vector x (A * x = b) */
    int *x = (int*)malloc(sizeof(int) * COLS);
    for (int i = 0; i < COLS; i++) x[i] = i;

    MPI_Barrier(*comm_grid);
    const double t0 = MPI_Wtime();

    /* Calculate partial solution */
    int *coords = (int*)malloc(sizeof(int) * 2);
    MPI_Cart_coords(*comm_grid, pid_comm_grid, 2, coords);
    int *partial_solution = calculate_partial_solution(local_matrix_block, x, ROWS, coords);

    /* Calculate total solution */
    int *total_solution = (int*)calloc(ROWS, sizeof(int));
    MPI_Allreduce(
        partial_solution,
        total_solution,
        ROWS,
        MPI_INT,
        MPI_SUM,
        *comm_grid
    );

    const double t1 = MPI_Wtime();

    double total_time = t1 - t0;
    double max_total_time;
    MPI_Reduce(&total_time, &max_total_time, 1, MPI_DOUBLE, MPI_MAX, 0, *comm_grid);

    /* Print results */
    if (pid_comm_grid == 0) {
        printf("Global matrix: \n");
        print_matrix(ROWS, COLS, global_matrix);

        printf("Vector x: ");
        print_vector(x, COLS);

        printf("Result: ");
        print_vector(total_solution, ROWS);

        printf("[TOTAL TIME] %e seconds.\n", max_total_time);
    }

    MPI_Barrier(*comm_grid);

    printf("\n[LOCAL MATRIX] Rank = %d \n", pid_comm_grid);
    print_matrix(local_matrix_block->rows, local_matrix_block->columns, local_matrix_block->data);

    free(total_solution);
    free(partial_solution);
    free(coords);
    free(local_matrix_block);
    free(x);
    free(comm_grid);

    return EXIT_SUCCESS;
}

void check_validity_cartesian_topology_processes(
    const int total_rows_matrix,
    const int total_columns_matrix,
    int total_processes,
    int comm_grid_total_rows,
    const int comm_grid_total_columns,
    int pid_comm_world
) {
    if (total_processes % comm_grid_total_rows != 0) {
        if (pid_comm_world == 0) {
            fprintf(stderr,
                "The total number of processes must be divisible by the number "
                "of rows of the cartesian topology of processes (argument <grid_comm_rows>)!\n"
            );
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (total_rows_matrix % comm_grid_total_rows != 0) {
        if (pid_comm_world == 0) {
            fprintf(stderr,
                "The total number of rows of the matrix must be divisible by the total number "
                "of rows of the cartesian topology of processes (argument <grid_comm_rows>)!\n"
            );
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (total_columns_matrix % comm_grid_total_columns != 0) {
        if (pid_comm_world == 0) {
            fprintf(stderr,
                "The total number of columns of the matrix must be divisible by the total number "
                "of columns of the cartesian topology of processes!\n"
            );
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
}

void print_vector(int *v, int size) {
    for (int i = 0; i < size; i++)
        printf("%s%d%s", i == 0 ? "[" : "", v[i], i == size - 1 ? "]\n" : " ");
}

void print_matrix(int rows, int columns, const int *matrix) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            printf("%3d ",(int)matrix[i * columns + j]);
        }
        printf("\n");
    }
    printf("\n");
}

void check_arguments(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Correct usage: %s <grid_comm_rows> <matrix_rows> <matrix_columns>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

void create_bidimensional_grid(
    MPI_Comm *comm_grid,
    const unsigned int rows,
    const unsigned int columns
) {
    static const int N_DIMS = 2;

    int *dims = (int*)malloc(sizeof(int) * N_DIMS);
    dims[0] = (int)rows;
    dims[1] = (int)columns;

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

int *calculate_partial_solution(MatrixBlock *matrix_block, const int *x, const int x_size, const int *comm_grid_coords) {
    int *partial_solution = (int*)calloc(x_size, sizeof(int));
    for (int i = 0; i < matrix_block->rows; i++) {
        for (int j = 0; j < matrix_block->columns; j++) {
            partial_solution[i + (matrix_block->rows * comm_grid_coords[0])] +=
                matrix_block->data[i * matrix_block->columns + j]  *
                x[j + (matrix_block->columns * comm_grid_coords[1])];
        }
    }
    return partial_solution;
}

int get_matrix_columns_command_line(char **argv) {
    const static int MATRIX_COLUMNS_ARGV_INDEX = 3;
    const static char *MATRIX_COLUMNS_ARGV_NAME = "<matrix_columns>";
    if (!is_number(argv[MATRIX_COLUMNS_ARGV_INDEX])) {
        fprintf(stderr, "%s must be a positive number!\n", MATRIX_COLUMNS_ARGV_NAME);
        exit(EXIT_FAILURE);
    }
    const int columns = atoi(argv[MATRIX_COLUMNS_ARGV_INDEX]);
    if (columns < 1) {
        fprintf(stderr, "%s must be greater than one!\n", MATRIX_COLUMNS_ARGV_NAME);
        exit(EXIT_FAILURE);
    }
    return columns;
}

int get_matrix_rows_command_line(char **argv) {
    const static int MATRIX_ROWS_ARGV_INDEX = 2;
    const static char *MATRIX_ROWS_ARGV_NAME = "<matrix_rows>";
    if (!is_number(argv[MATRIX_ROWS_ARGV_INDEX])) {
        fprintf(stderr, "%s must be a positive number!\n", MATRIX_ROWS_ARGV_NAME);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    const int rows = atoi(argv[MATRIX_ROWS_ARGV_INDEX]);
    if (rows < 1) {
        fprintf(stderr, "%s must be greater than one!\n", MATRIX_ROWS_ARGV_NAME);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    return rows;
}

int get_number_of_comm_grid_rows_command_line(char **argv, const unsigned int total_processes) {
    const static int GRID_COMM_ROWS_ARGV_INDEX = 1;
    const static char *GRID_COMM_ROWS_ARGV_NAME = "<grid_comm_rows>";
    if (!is_number(argv[GRID_COMM_ROWS_ARGV_INDEX])) {
        fprintf(stderr, "%s must be a positive number!\n", GRID_COMM_ROWS_ARGV_NAME);
        exit(EXIT_FAILURE);
    }
    int grid_comm_rows = atoi(argv[GRID_COMM_ROWS_ARGV_INDEX]);
    if (grid_comm_rows < 1) {
        fprintf(stderr, "%s must be greater than one!\n", GRID_COMM_ROWS_ARGV_NAME);
        exit(EXIT_FAILURE);
    }
    if (grid_comm_rows > total_processes) {
        fprintf(
            stderr,
            "%s can't be greater than the number of processes %d!\n",
            GRID_COMM_ROWS_ARGV_NAME, total_processes
        );
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    return grid_comm_rows;
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
