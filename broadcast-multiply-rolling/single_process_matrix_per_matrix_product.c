#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <time.h>

typedef struct {
    int rows;
    int columns;
    int *data;
} MatrixBlock;

int get_square_matrix_dim_command_line(char **argv);
MatrixBlock *multiply_matrices(MatrixBlock *A, MatrixBlock *B);
bool is_number(const char *s);
MatrixBlock *multiply_matrices(MatrixBlock *A, MatrixBlock *B);
void print_matrix(int rows, int columns, int *matrix);

int main(int argc, char **argv) {
    const int square_matrix_dim = get_square_matrix_dim_command_line(argv);
    const int square_matrix_total_elements = square_matrix_dim * square_matrix_dim;

    /* Create and fill the global matrix A and B */
    int A[square_matrix_total_elements];
    int B[square_matrix_total_elements];
    for (int i = 0; i < square_matrix_total_elements; i++) {
        A[i] = i;
        B[i] = i;
    }

    MatrixBlock* matrix_blocks = (MatrixBlock*) malloc(sizeof(MatrixBlock) * 2);
    matrix_blocks[0].data = A;
    matrix_blocks[1].data = B;
    matrix_blocks[0].rows = square_matrix_dim;
    matrix_blocks[1].rows = square_matrix_dim;
    matrix_blocks[0].columns = square_matrix_dim;
    matrix_blocks[1].columns = square_matrix_dim;

    struct timeval time;
    double t0, t1;

    double time_spent = 0.0;
    clock_t begin = clock();

    // Calculates solution
    MatrixBlock *C = multiply_matrices(&matrix_blocks[0], &matrix_blocks[1]);

    clock_t end = clock();

    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;

    printf("[TOTAL TIME SINGLE PROCESS] %f seconds.\n", time_spent);

    return EXIT_SUCCESS;
}

int get_square_matrix_dim_command_line(char **argv) {
    const static int SQUARE_MATRIX_DIM_ARGV_INDEX = 1;
    const static char *SQUARE_MATRIX_DIM_ARGV_NAME = "<square_matrix_dim>";
    if (!is_number(argv[SQUARE_MATRIX_DIM_ARGV_INDEX])) {
        fprintf(stderr, "%s must be a positive number!\n", SQUARE_MATRIX_DIM_ARGV_NAME);
        exit(EXIT_FAILURE);
    }
    const int dim = atoi(argv[SQUARE_MATRIX_DIM_ARGV_INDEX]);
    if (dim < 1) {
        fprintf(stderr, "%s must be greater than one!\n", SQUARE_MATRIX_DIM_ARGV_NAME);
        exit(EXIT_FAILURE);
    }
    return dim;
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

void print_matrix(int rows, int columns, int *matrix) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            printf("%3d ",(int)matrix[i * columns + j]);
        }
        printf("\n");
    }
    printf("\n");
}
