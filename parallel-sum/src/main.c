#include "../include/preconditions.h"
#include "../include/parallelsum.h"

const static unsigned int N_PARAMETERS_BEFORE_NUMBERS = 3;

int main(int argc, char **argv) {
    check_number_parameters(argc, argv);
    check_the_parameters_are_all_numbers(argc, argv);

    const int strategy_id = atoi(argv[1]);
    check_strategy_id(strategy_id, argv);

    int tmp_root_pid = atoi(argv[2]);
    const int root_pid = (tmp_root_pid == -1 ? 0 : tmp_root_pid);
    const bool everyone_must_print_the_result = (tmp_root_pid == -1);

    MPI_Init(&argc, &argv);

    int total_number_of_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &total_number_of_processes);
    check_root_pid(root_pid, total_number_of_processes);

    const int total_numbers = argc - N_PARAMETERS_BEFORE_NUMBERS;
    check_total_number_of_processes_less_than_or_equal_to_the_total_numbers(
        total_number_of_processes, 
        total_numbers
    );

    int this_pid;
    MPI_Comm_rank(MPI_COMM_WORLD, &this_pid);

    const int number_of_locations = ceil(total_numbers / total_number_of_processes);
    const int start = (this_pid * number_of_locations) + N_PARAMETERS_BEFORE_NUMBERS;
    const int end   = (start + number_of_locations);
    int sum = computes_local_sum(argv, start, end);

    switch (strategy_id) {
        case STRATEGY_ONE:
            computes_strategy_one(&sum, this_pid, root_pid, total_number_of_processes);
            break;
        case STRATEGY_TWO:
            computes_strategy_two(&sum, this_pid, total_number_of_processes);
            break;
        case STRATEGY_THREE:
            computes_strategy_three(&sum, this_pid, total_number_of_processes);
            break;
        default:
            return EXIT_FAILURE;
    }

    if (everyone_must_print_the_result || this_pid == root_pid) {
        printf("[PID %d] Result: %d\n", this_pid, sum);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}