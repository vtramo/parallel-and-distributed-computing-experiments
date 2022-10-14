#include "../include/parallelsum.h"

int computes_local_sum(char **argv, const int start, const int end) {
    int sum = 0;
    for (int i = start; i < end; i++) {
        sum += atoi(argv[i]);
    }
    return sum;
}

void computes_strategy_one(
    int *sum, 
    const int this_pid, 
    const int root_pid, 
    const int total_number_of_processes
) {
    if (this_pid == root_pid) {
        for (int pid = 0; pid < total_number_of_processes; pid++) {
            if (pid == this_pid) continue;
            const int tag = MESSAGE_TAG + pid;
            int partial_sum;
            MPI_Recv(&partial_sum, 1, MPI_INT, pid, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);       
            *sum += partial_sum;
        }
    } else {
        const int tag = MESSAGE_TAG + this_pid;
        MPI_Send(sum, 1, MPI_INT, root_pid, tag, MPI_COMM_WORLD);
    }
}