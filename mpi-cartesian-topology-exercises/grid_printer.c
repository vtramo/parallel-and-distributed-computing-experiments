#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>

/*
 *  GRID PRINTER - EXERCISE 1
 *
 *  Given P processes and a positive integer p, creates a grid of p x q
 *  processes, where p x q = P, in which each process prints its
 *  coordinates on standard output.
 *
 *  Preconditions:
 *  - p <= P
 */

int get_number_of_comm_grid_rows_command_line(char **argv, unsigned int total_processes);
MPI_Comm *create_bidimensional_grid(unsigned int rows, unsigned int columns);
bool is_number(const char *s);

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int number_of_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);

    int pid_comm_world;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid_comm_world);

    int p = (pid_comm_world == 0) ? get_number_of_comm_grid_rows_command_line(argv, number_of_processes) : 0;

    MPI_Bcast(&p, 1, MPI_INT, 0, MPI_COMM_WORLD);

    const int q = number_of_processes / p;

    const MPI_Comm *comm_grid = create_bidimensional_grid(p, q);

    int pid_comm_grid;
    MPI_Comm_rank(*comm_grid, &pid_comm_grid);

    int *coordinates = (int*)malloc(sizeof(int) * 2);
    MPI_Cart_coords(*comm_grid, pid_comm_grid, 2, coordinates);

    printf("[PROCESSOR %d] Grid coordinates (%d, %d)\n", pid_comm_grid, coordinates[0], coordinates[1]);

    MPI_Finalize();
    return 0;
}

MPI_Comm *create_bidimensional_grid(
    const unsigned int rows,
    const unsigned int columns
) {
    static const int N_DIMS = 2;
    MPI_Comm *comm_grid = (MPI_Comm*) malloc(sizeof(MPI_Comm));

    int *dims = (int*)malloc(sizeof(int) * N_DIMS);
    dims[0] = (int)rows;
    dims[1] = (int)columns;

    const int *periods = (int*)calloc(N_DIMS, sizeof(int));
    const int reorder = 0;

    MPI_Cart_create(MPI_COMM_WORLD, N_DIMS, dims, periods, reorder, comm_grid);
    return comm_grid;
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