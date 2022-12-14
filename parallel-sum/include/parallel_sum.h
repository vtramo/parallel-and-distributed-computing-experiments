#include <mpi.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

enum Strategy {
    STRATEGY_ONE = 1,
    STRATEGY_TWO = 2,
    STRATEGY_THREE = 3
};

static const int MESSAGE_TAG = 80;

int computes_local_sum(const int *numbers, const int start, const int end);

void computes_strategy_one(
    int *sum, 
    const int this_pid, 
    const int root_pid, 
    const int total_number_of_processes
);

void computes_strategy_two(
    int *sum,
    const int this_pid,
    const int total_number_of_processes
);

void computes_strategy_three(
    int *sum,
    const int this_pid,
    const int total_number_of_processes
);

typedef struct {
    unsigned int size;
    int* numbers;
} Numbers;

Numbers* read_numbers_from_file(char *file_name);