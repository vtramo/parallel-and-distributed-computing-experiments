#include "/homes/DMA/PDC/2022/TRMVCN99Y/parallel_sum_project/parallel_sum/include/parallel_sum.h"

int computes_local_sum(const int *numbers, const int start, const int end) {
    int sum = 0;
    for (int i = start; i < end; i++) {
        sum += numbers[i];
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

void computes_strategy_two(
    int *sum,
    const int this_pid,
    const int total_number_of_processes
) {
    for (int i = 0; i < (int)log2(total_number_of_processes); i++) {
        if (this_pid % (int)pow(2, i) == 0) {
            if (this_pid % (int)pow(2, i + 1) == 0) {
                const int pid_sender = this_pid + pow(2, i);
                const int tag = MESSAGE_TAG + pid_sender;
                int partial_sum;
                MPI_Recv(&partial_sum, 1, MPI_INT, pid_sender, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                *sum += partial_sum;
            } else {
                const int tag = MESSAGE_TAG + this_pid;
                const int pid_receiver = this_pid - pow(2, i);
                MPI_Send(sum, 1, MPI_INT, pid_receiver, tag, MPI_COMM_WORLD);
            }
        }
    }
}

void computes_strategy_three(
    int *sum,
    const int this_pid,
    const int total_number_of_processes
) {
    for (int i = 0; i < (int)log2(total_number_of_processes); i++) {
        const int pid_receiver = (this_pid % (int)pow(2, i + 1) < (int)pow(2, i))
            ? this_pid + pow(2, i)
            : this_pid - pow(2, i);
        const int tag_receiver  = MESSAGE_TAG + pid_receiver;
        const int tag_sender    = MESSAGE_TAG + this_pid;
        int partial_sum;
        MPI_Request request;
        MPI_Isend(sum, 1, MPI_INT, pid_receiver, tag_receiver, MPI_COMM_WORLD, &request);
        MPI_Request_free(&request);
        MPI_Recv(&partial_sum, 1, MPI_INT, pid_receiver, tag_sender, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        *sum += partial_sum;
    }
}

Numbers* read_numbers_from_file(char *file_name) {
    FILE *file = fopen(file_name, "r");
    int n = 0;
    int total_numbers = 0;
    int index = 0;
    int dim = 1;
    int *numbers = malloc(sizeof(int));
    while (fscanf(file, "%d", &n) > 0) {
        total_numbers++;
        numbers[index++] = n;
        numbers = realloc(numbers, sizeof(int) * ++dim);
    }
    fclose(file);
    Numbers *bean_numbers = malloc(sizeof(numbers));
    bean_numbers->size = total_numbers;
    bean_numbers->numbers = numbers;
    return bean_numbers;
}
