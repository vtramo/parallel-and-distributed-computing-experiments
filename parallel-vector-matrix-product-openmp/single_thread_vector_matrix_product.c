#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>

double **read_matrix_from_file(int rows, int columns, char *file_name);
double *read_vector_from_file(char *file_name);

void print_vector(double *vector, int size);

void free_matrix(double **A, int rows);

int main(int argc, char **argv) {
    const int rows = atoi(argv[1]);
    const int columns = atoi(argv[2]);

    char *matrix_file_name = argv[3];
    char *vector_file_name = argv[4];

    double **A = read_matrix_from_file(rows, columns, matrix_file_name);
    double *x = read_vector_from_file(vector_file_name);

    struct timeval time;
    double t0, t1;

    gettimeofday(&time, NULL);
    t0 = time.tv_sec + (time.tv_usec / 100000.0);

    /* Compute solution */
    double *b = (double*) malloc(sizeof(double) * rows);
    for (int i = 0; i < rows; i++) {
        double row_result = 0;
        for (int j = 0; j < columns; j++) {
            row_result += A[i][j] * x[j];
        }
        b[i] = row_result;
    }

    gettimeofday(&time, NULL);
    t1 = time.tv_sec + (time.tv_usec / 100000.0);

    printf("[TOTAL TIME SINGLE THREAD] %e seconds.\n", t1 - t0);

    free(x);
    free_matrix(A, rows);

    return EXIT_SUCCESS;
}

double **read_matrix_from_file(const int rows, const int columns, char *file_name) {
    FILE *file = fopen(file_name, "r");
    double **A = (double**) malloc(rows * sizeof(double*));
    double n = 0;
    for (int i = 0; i < rows; i++) {
        double *row = (double*) malloc(columns * sizeof(double));
        for (int j = 0; j < columns; j++) {
            if (fscanf(file, "%lf", &n) <= 0) return NULL;
            row[j] = n;
        }
        A[i] = row;
    }
    fclose(file);
    return A;
}

double* read_vector_from_file(char *file_name) {
    FILE *file = fopen(file_name, "r");
    double n = 0;
    int i = 0;
    int dim = 1;
    double *vector = malloc(sizeof(double));
    while (fscanf(file, "%lf", &n) > 0) {
        vector[i++] = n;
        vector = realloc(vector, sizeof(double) * ++dim);
    }
    fclose(file);
    return vector;
}

void print_vector(double *vector, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%f%s", vector[i], i + 1 == size ? "]\n" : ", ");
    }
}

void free_matrix(double **A, int rows) {
    for (int i = 0; i < rows; i++) {
        free(A[i]);
    }
    free(A);
}

