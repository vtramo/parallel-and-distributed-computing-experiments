#include <mpi.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
 *  MATRIX PARTITIONING BLOCKS - EXERCISE 3
 *
 *  Given P processes and a matrix M of dimensions RxC, creates a Cartesian topology
 *  of processes of dimensions pxq. It then identifies pxq sub-blocks of the matrix M
 *  and assigns them to each process that has the corresponding coordinates.
 *
 *  Preconditions:
 *  - RxC >= P
 */

typedef struct {
    int rows;
    int columns;
    char *data;
} MatrixBlock;

MatrixBlock* partition_and_distribute_matrix_blocks(
    const MPI_Comm *comm_grid,
    int matrix_rows,
    int matrix_columns,
    char *global_matrix
);

MPI_Comm *create_bidimensional_grid(unsigned int rows, unsigned int columns);

void print_matrix(int rows, int columns, const char *matrix);

int get_matrix_columns_command_line(char **argv);
int get_matrix_rows_command_line(char **argv);
int get_number_of_comm_grid_rows_command_line(char **argv, unsigned int total_processes);
int get_number_of_rows_stdin(unsigned int number_of_processes);
bool is_number(const char *s);
void check_arguments(int argc, char **argv);

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

    int comm_grid_rows = (pid_comm_world == 0) ? get_number_of_comm_grid_rows_command_line(argv, total_processes) : 0;

    MPI_Bcast(&comm_grid_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);

    const int comm_grid_columns = total_processes / comm_grid_rows;

    const MPI_Comm *comm_grid = create_bidimensional_grid(comm_grid_rows, comm_grid_columns);

    int pid_comm_grid;
    MPI_Comm_rank(*comm_grid, &pid_comm_grid);

    char global_matrix[ROWS * COLS];
    if (pid_comm_grid == 0) {
        for (int i = 0; i < ROWS * COLS; i++) {
            global_matrix[i] = (char)i;
        }
    }

    MatrixBlock *local_matrix_block = partition_and_distribute_matrix_blocks(comm_grid, ROWS, COLS, global_matrix);
    const int block_rows = local_matrix_block->rows;
    const int block_columns = local_matrix_block->columns;
    const char *local_matrix = local_matrix_block->data;

    if (pid_comm_grid == 0) {
        printf("Global matrix: \n");
        print_matrix(ROWS, COLS, global_matrix);
    }

    MPI_Barrier(*comm_grid);

    printf("\n[LOCAL MATRIX] Rank = %d \n", pid_comm_grid);
    print_matrix(block_rows, block_columns, local_matrix);
}

void print_matrix(int rows, int columns, const char *matrix) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            printf("%3d ",(int)matrix[i * columns + j]);
        }
        printf("\n");
    }
    printf("\n");
}

void check_arguments(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Correct usage: %s <grid_comm_rows> <matrix_rows> <matrix_columns>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

MPI_Comm *create_bidimensional_grid(
    const unsigned int rows,
    const unsigned int columns
) {
    static const int N_DIMS = 2;
    MPI_Comm *comm_grid = (MPI_Comm*)malloc(sizeof(MPI_Comm));

    int *dims = (int*)malloc(sizeof(int) * N_DIMS);
    dims[0] = (int)rows;
    dims[1] = (int)columns;

    const int *periods = (int*)calloc(N_DIMS, sizeof(int));
    const int reorder = 0;

    MPI_Cart_create(MPI_COMM_WORLD, N_DIMS, dims, periods, reorder, comm_grid);
    return comm_grid;
}

MatrixBlock* partition_and_distribute_matrix_blocks(
    const MPI_Comm *comm_grid,
    const int matrix_rows,
    const int matrix_columns,
    char *global_matrix
) {
    int pid_comm_grid;
    MPI_Comm_rank(*comm_grid, &pid_comm_grid);

    int dims[2], periods[2], coords[2];
    MPI_Cart_get(*comm_grid, 2, dims, periods, coords);
    const int comm_grid_rows = dims[0];
    const int comm_grid_columns = dims[1];
    const int total_processes = comm_grid_rows * comm_grid_columns;

    const int block_rows = matrix_rows / comm_grid_rows;
    const int block_columns = matrix_columns / comm_grid_columns;

    if (pid_comm_grid == 0) {
        for (int i = 0; i < matrix_rows * matrix_columns; i++) {
            global_matrix[i] = (char)i;
        }
    }

    char *local_matrix = (char*)malloc(sizeof(char) * block_rows * block_columns);

    MPI_Datatype blocktype_not_resized;
    MPI_Datatype blocktype_resized;

    MPI_Type_vector(block_rows, block_columns, matrix_columns, MPI_CHAR, &blocktype_not_resized);
    MPI_Type_create_resized( blocktype_not_resized, 0, sizeof(char), &blocktype_resized);
    MPI_Type_commit(&blocktype_resized);

    int displs[total_processes];
    int send_counts[total_processes];
    for (int i = 0; i < comm_grid_rows; i++) {
        for (int j = 0; j < comm_grid_columns; j++) {
            displs[i * comm_grid_columns + j] = i * matrix_columns * block_rows + j * block_columns;
            send_counts[i * comm_grid_columns + j] = 1;
        }
    }

    MPI_Scatterv(global_matrix, send_counts, displs, blocktype_resized, local_matrix, block_rows * block_columns, MPI_CHAR, 0, MPI_COMM_WORLD);

    MatrixBlock *matrix_block = (MatrixBlock*)malloc(sizeof(MatrixBlock));
    matrix_block->columns = block_columns;
    matrix_block->rows = block_rows;
    matrix_block->data = local_matrix;
    return matrix_block;
}

int get_number_of_rows_stdin(const unsigned int number_of_processes) {
    int rows;

    bool is_valid_input = false;
    do {
        printf("Enter the number 'p' of rows: ");
        scanf("%d", &rows);

        if (rows > number_of_processes) {
            printf("\nThe number of rows can't be greater than the number of processes!\n\n");
        } else {
            is_valid_input = true;
        }
    } while(!is_valid_input);

    return rows;
}

int get_matrix_columns_command_line(char **argv) {
    const static int MATRIX_COLUMNS_ARGV_INDEX = 4;
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
    const static int MATRIX_ROWS_ARGV_INDEX = 3;
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
    const static int GRID_COMM_ROWS_ARGV_INDEX = 2;
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
