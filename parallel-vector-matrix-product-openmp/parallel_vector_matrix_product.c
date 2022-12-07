#include <omp.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

double *matxvet(int rows, int columns, const double *restrict x, double **restrict A);

double **read_matrix_from_file(int rows, int columns, char *file_name);
double *read_vector_from_file(char *file_name);

int get_number_of_rows_command_line(char **argv);
int get_number_of_columns_command_line(char **argv);
bool get_printing_mode(char **argv);

void check_arguments(int argc, char **argv);
bool is_number(const char *s);
void print_correct_usage(char **argv);

void print_vector(double *vector, int size);
void print_matrix(double **A, int rows, int columns);

void free_matrix(double **A, int rows);

int main(int argc, char **argv) {
    check_arguments(argc, argv);

    const bool is_active_printing_mode = get_printing_mode(argv);
    const int rows = get_number_of_rows_command_line(argv);
    const int columns = get_number_of_columns_command_line(argv);

    char *matrix_file_name = argv[3];
    char *vector_file_name = argv[4];

    double **A = read_matrix_from_file(rows, columns, matrix_file_name);
    double *x = read_vector_from_file(vector_file_name);

    struct timeval time;
    double t0, t1;

    gettimeofday(&time, NULL);
    t0 = time.tv_sec + (time.tv_usec / 100000.0);

    /* Compute solution */
    double *b = matxvet(rows, columns, x, A);

    gettimeofday(&time, NULL);
    t1 = time.tv_sec + (time.tv_usec / 100000.0);


    /* Print result */
    if (is_active_printing_mode) {
        printf("Matrix:\n\n");
        print_matrix(A, rows, columns);
        printf("\n\nVector x: ");
        print_vector(x, columns);
        printf("\nResult: ");
        print_vector(b, rows);
    }

    printf("[TOTAL TIME PARALLEL] %e seconds.\n", t1 - t0);

    free(x);
    free_matrix(A, rows);

    return EXIT_SUCCESS;
}

double *matxvet(int rows, int columns, const double *restrict x, double **restrict A) {
    double *b = (double*) calloc(rows, sizeof(double));
    int i, j;

    #pragma omp parallel for default(none) shared(rows, columns, A, x, b) private(i, j)
    for (i = 0; i < rows; i++) {
        for (j = 0; j < columns; j++) {
            b[i] += A[i][j] * x[j];
        }
    }

    return b;
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

void check_arguments(const int argc, char **argv) {
    if (argc != 6 || !is_number(argv[1]) || !is_number(argv[2])) {
        print_correct_usage(argv);
        exit(EXIT_FAILURE);
    }
}

int get_number_of_rows_command_line(char **argv) {
    const static int NO_ROWS_INDEX_COMM_LINE = 1;
    if (!is_number(argv[NO_ROWS_INDEX_COMM_LINE])) {
        print_correct_usage(argv);
        exit(EXIT_FAILURE);
    }
    return atoi(argv[NO_ROWS_INDEX_COMM_LINE]);
}

int get_number_of_columns_command_line(char **argv) {
    const static int NO_COLUMNS_INDEX_COMM_LINE = 2;
    if (!is_number(argv[NO_COLUMNS_INDEX_COMM_LINE])) {
        print_correct_usage(argv);
        exit(EXIT_FAILURE);
    }
    return atoi(argv[NO_COLUMNS_INDEX_COMM_LINE]);
}

bool get_printing_mode(char **argv) {
    const static int PRINTING_MODE_INDEX_COMM_LINE = 5;
    char *printing_mode = argv[PRINTING_MODE_INDEX_COMM_LINE];
    if (strcmp(printing_mode, "true") != 0 && strcmp(printing_mode, "false") != 0) {
        print_correct_usage(argv);
        exit(EXIT_FAILURE);
    }
    return strcmp(printing_mode, "true") == 0;
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

void print_correct_usage(char **argv) {
    printf("Correct usage: %s <rows> <columns> <file_matrix> <file_vector> <printing_mode_boolean_value>\n", argv[0]);
}

void print_matrix(double **A, int rows, int columns) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            printf("%f%s", A[i][j], j + 1 == columns ? "\n" : " ");
        }
    }
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