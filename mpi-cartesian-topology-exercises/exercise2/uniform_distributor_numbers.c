#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

/*
 *  UNIFORM DISTRIBUTOR NUMBERS - EXERCISE 2
 *
 *  Given P processes and a vector V of dimension N, equally distributes the elements of V among the processes.
 *
 *  Preconditions:
 *  - N >= P
 */

#define MAX_RANDOM_NUMBER 11

enum {
    UNIFORM_DISTRIBUTOR_NUMBERS_V1 = 1,
    UNIFORM_DISTRIBUTOR_NUMBERS_V2 = 2,
    UNIFORM_DISTRIBUTOR_NUMBERS_V3 = 3,
};

typedef struct {
    int size;
    int *data;
} IntVector;

/* VERSION 1: with MPI_Scatter */
IntVector* uniform_distributor_numbers_v1(MPI_Comm comm, int *numbers, int total_numbers);

/* VERSION 2: with MPI_Isend() and MPI_Recv() */
IntVector* uniform_distributor_numbers_v2(MPI_Comm comm, int *numbers, int total_numbers);

/* VERSION 3: with MPI_Scatterv() */
IntVector* uniform_distributor_numbers_v3(MPI_Comm comm, int *numbers, int total_numbers);

int* generate_random_numbers(unsigned int total, unsigned int max);
void print_vector(IntVector *vector);
void print_vector_s(int *vector, int size);
void check_arguments(int argc, char **argv);
bool is_number(const char *s);
const int get_size_vector_command_line(char **argv, int total_processes);
const int get_uniform_distributor_strategy_command_line(char **argv);

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    check_arguments(argc, argv);

    int total_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &total_processes);

    int this_pid;
    MPI_Comm_rank(MPI_COMM_WORLD, &this_pid);

    const int size_vector = get_size_vector_command_line(argv, total_processes);
    int *global_vector = generate_random_numbers(size_vector, MAX_RANDOM_NUMBER);

    if (this_pid == 0) {
        printf("Global Vector:\n - Size: %d\n - Numbers: ", size_vector);
        print_vector_s(global_vector, size_vector);
    }

    const int strategy = get_uniform_distributor_strategy_command_line(argv);
    IntVector *local_vector = NULL;

    MPI_Barrier(MPI_COMM_WORLD);
    const double t0 = MPI_Wtime();

    switch (strategy) {
        case UNIFORM_DISTRIBUTOR_NUMBERS_V1:
            local_vector = uniform_distributor_numbers_v1(MPI_COMM_WORLD, global_vector, size_vector);
            break;
        case UNIFORM_DISTRIBUTOR_NUMBERS_V2:
            local_vector = uniform_distributor_numbers_v2(MPI_COMM_WORLD, global_vector, size_vector);
            break;
        case UNIFORM_DISTRIBUTOR_NUMBERS_V3:
            local_vector = uniform_distributor_numbers_v3(MPI_COMM_WORLD, global_vector, size_vector);
            break;
    }

    const double t1 = MPI_Wtime();

    double total_time = t1 - t0;
    double max_total_time;
    MPI_Reduce(&total_time, &max_total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (this_pid == 0) {
        printf("[TOTAL TIME] %e seconds.\n", max_total_time);
    }

    printf("[PID %d]: ", this_pid);
    print_vector(local_vector);

    MPI_Finalize();

    return 0;
}

/* with MPI_Scatter */
IntVector* uniform_distributor_numbers_v1(MPI_Comm comm, int *numbers, int total_numbers) {
    int total_processes;
    MPI_Comm_size(comm, &total_processes);

    int this_pid;
    MPI_Comm_rank(comm, &this_pid);

    const int number_of_locations_per_process = total_numbers / total_processes;
    const int rest = total_numbers % total_processes;

    const int linear_vector_size = total_numbers + (rest != 0 ? (total_processes - rest) : 0);
    int *linear_vector = (int*)calloc(linear_vector_size, sizeof(int));
    int offset_linear_vector = 0;

    if (this_pid == 0) {
        for (int pid = 0; pid < total_processes; pid++) {
            int offset_rest_start = 0, offset_rest_end = 0;

            if (rest != 0) {
                if (pid == 0) {
                    offset_rest_end = 1;
                } else if (pid <= rest) {
                    offset_rest_start = pid;
                    offset_rest_end = ((pid < rest) ? 1 : 0);
                } else {
                    offset_rest_start = rest;
                    offset_linear_vector++;
                }
            }

            const int start_index = (pid * number_of_locations_per_process) + offset_rest_start;
            const int end_index = (start_index + number_of_locations_per_process) + offset_rest_end;
            
            memcpy(
                &linear_vector[start_index + offset_linear_vector],
                &numbers[start_index],
                sizeof(int) * (end_index - start_index)
            );
        }
    }

    const int locations_size = number_of_locations_per_process + ((rest == 0) ? 0 : 1);
    int *my_vector = (int*)calloc(locations_size, sizeof(int));
    MPI_Scatter(
        linear_vector,
        locations_size,
        MPI_INT,
        my_vector,
        locations_size,
        MPI_INT,
        0,
        comm
    );
    
    free(linear_vector);

    IntVector *int_vector = (IntVector*)malloc(sizeof(IntVector));
    int_vector->data = my_vector;
    int_vector->size = this_pid < rest ? number_of_locations_per_process + 1 : number_of_locations_per_process;
    return int_vector;
}

/* with MPI_Isend() and MPI_Recv() */
IntVector* uniform_distributor_numbers_v2(MPI_Comm comm, int *numbers, int total_numbers) {
    int total_processes;
    MPI_Comm_size(comm, &total_processes);

    int this_pid;
    MPI_Comm_rank(comm, &this_pid);

    const int number_of_locations_per_process = total_numbers / total_processes;
    const int rest = total_numbers % total_processes;

    static const int TAG_NUM = 80;

    if (this_pid == 0) {
        for (int pid = 0; pid < total_processes; pid++) {

            const int offset_rest_start = (rest != 0 && pid <= rest)
                ? pid
                : rest;

            const int start_index = (pid * number_of_locations_per_process) + offset_rest_start;
            
            MPI_Request req;
            MPI_Isend(
                &numbers[start_index],
                pid >= rest ? number_of_locations_per_process : number_of_locations_per_process + 1,
                MPI_INT,
                pid,
                pid + TAG_NUM,
                comm,
                &req
            );
            MPI_Request_free(&req);
        }
    }

    int *my_vector = (int*)calloc(number_of_locations_per_process + 1, sizeof(int));
    MPI_Recv(
        my_vector,
        this_pid >= rest ? number_of_locations_per_process : number_of_locations_per_process + 1,
        MPI_INT,
        0,
        this_pid + TAG_NUM,
        comm,
        MPI_STATUS_IGNORE
    );

    IntVector *int_vector = (IntVector*)malloc(sizeof(IntVector));
    int_vector->data = my_vector;
    int_vector->size = this_pid >= rest ? number_of_locations_per_process : number_of_locations_per_process + 1;
    return int_vector;
}

/* with MPI_Scatterv() */
IntVector* uniform_distributor_numbers_v3(MPI_Comm comm, int *numbers, int total_numbers) {
    int total_processes;
    MPI_Comm_size(comm, &total_processes);

    int this_pid;
    MPI_Comm_rank(comm, &this_pid);

    const int number_of_locations_per_process = total_numbers / total_processes;
    int rest = total_numbers % total_processes;

    int *sendcounts = (int*)malloc(sizeof(int) * total_processes);
    int *displs = (int*)calloc(total_processes, sizeof(int));
    int total_data_send = 0;

    if (this_pid == 0) {
        for (int pid = 0; pid < total_processes; pid++) {
            sendcounts[pid] = number_of_locations_per_process;

            if (rest > 0) {
                sendcounts[pid]++;
                rest--;
            }

            displs[pid] = total_data_send;
            total_data_send += sendcounts[pid];
        }
    }

    int *my_vector = (int*)calloc(number_of_locations_per_process + 1, sizeof(int));
    MPI_Scatterv(
        numbers,
        sendcounts,
        displs,
        MPI_INT,
        my_vector,
        number_of_locations_per_process + 1,
        MPI_INT,
        0,
        comm
    );

    IntVector *int_vector = (IntVector*)malloc(sizeof(IntVector));
    int_vector->data = my_vector;
    int_vector->size = sendcounts[this_pid];

    free(sendcounts);
    free(displs);

    return int_vector;
}

void print_vector(IntVector *vector) {
    for (int i = 0; i < vector->size; i++)
        printf("%d ", vector->data[i]);
    printf("\n");
}

void print_vector_s(int *vector, int size) {
    for (int i = 0; i < size; i++)
        printf("%d ", vector[i]);
    printf("\n");
}

int* generate_random_numbers(const unsigned int total, const unsigned int max) {
    srand(time(0));
    int *random_numbers = malloc(sizeof(int) * total);
    for (int i = 0; i < total; i++) {
        random_numbers[i] = rand() % max;
        if (random_numbers[i] % 2 == 0)
            random_numbers[i] = -random_numbers[i];
    }
    return random_numbers;
}

const int get_uniform_distributor_strategy_command_line(char **argv) {
    const static int UNIFORM_DISTRIBUTOR_STRATEGY_ARGV_INDEX = 3;
    const static char *SIZE_VECTOR_ARGV_NAME = "<strategy>";
    if (!is_number(argv[UNIFORM_DISTRIBUTOR_STRATEGY_ARGV_INDEX])) {
        fprintf(stderr, "%s must be a number in {1, 2, 3}!\n", SIZE_VECTOR_ARGV_NAME);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    const int strategy = atoi(argv[UNIFORM_DISTRIBUTOR_STRATEGY_ARGV_INDEX]);
    if (strategy != UNIFORM_DISTRIBUTOR_NUMBERS_V1 && strategy != UNIFORM_DISTRIBUTOR_NUMBERS_V2 && strategy != UNIFORM_DISTRIBUTOR_NUMBERS_V3) {
        fprintf(stderr, "%s must be a number in {1, 2, 3}!\n", SIZE_VECTOR_ARGV_NAME);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    return strategy;
}

const int get_size_vector_command_line(char **argv, int total_processes) {
    const static int SIZE_VECTOR_ARGV_INDEX = 2;
    const static char *SIZE_VECTOR_ARGV_NAME = "<total_random_numbers>";
    if (!is_number(argv[SIZE_VECTOR_ARGV_INDEX])) {
        fprintf(stderr, "%s must be a positive number!\n", SIZE_VECTOR_ARGV_NAME);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    const int size_vector = atoi(argv[SIZE_VECTOR_ARGV_INDEX]);
    if (size_vector < total_processes) {
        fprintf(stderr, "%s must be greater than total processes %d!\n", SIZE_VECTOR_ARGV_NAME, total_processes);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    return size_vector;
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

void check_arguments(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Correct usage: %s <total_random_numbers> <strategy_number>\n", argv[0]);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
}