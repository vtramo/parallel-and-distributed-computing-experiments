#include "/homes/DMA/PDC/2022/TRMVCN99Y/parallel_sum_project/parallel_sum/include/preconditions.h"
#include "/homes/DMA/PDC/2022/TRMVCN99Y/parallel_sum_project/parallel_sum/include/parallel_sum.h"
#include <float.h>

typedef struct {
    unsigned int start;
    unsigned int end;
} RangeExtremes;

RangeExtremes* computes_range_extremes(
    unsigned int total_number_of_processes,
    unsigned int total_numbers,
    unsigned int this_pid
);

int main(int argc, char **argv) {
    check_number_parameters(argc, argv);
    check_parameters(argv);

    int tmp_root_pid = atoi(argv[2]);
    const int root_pid = (tmp_root_pid == -1 ? 0 : tmp_root_pid);
    const bool everyone_must_print_the_result = (tmp_root_pid == -1);

    MPI_Init(&argc, &argv);

    int total_number_of_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &total_number_of_processes);
    check_root_pid(root_pid, total_number_of_processes);

    char *file_path_numbers = argv[3];
    const Numbers *numbers_bean = read_numbers_from_file(file_path_numbers);
    const int total_numbers = numbers_bean->size;
    const int *numbers = numbers_bean->numbers;
    check_total_number_of_processes_less_than_or_equal_to_the_total_numbers(
        total_number_of_processes, 
        total_numbers
    );

    const int strategy_id = atoi(argv[1]);
    check_strategy_id(strategy_id, total_number_of_processes, argv[0]);

    int this_pid;
    MPI_Comm_rank(MPI_COMM_WORLD, &this_pid);

    RangeExtremes *range_extremes = computes_range_extremes(
        total_number_of_processes,
        total_numbers,
        this_pid
    );
    const int start = range_extremes->start;
    const int end   = range_extremes->end;
    free(range_extremes);

	MPI_Barrier(MPI_COMM_WORLD);
    const double t0 = MPI_Wtime();

    int sum = computes_local_sum(numbers, start, end);

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

    const double t1 = MPI_Wtime();

    if (everyone_must_print_the_result || this_pid == root_pid) {
        printf("[PID %d] Result: %d\n", this_pid, sum);
    }

    double total_time = t1 - t0;
    double max_total_time;

    MPI_Reduce(&total_time, &max_total_time, 1, MPI_DOUBLE, MPI_MAX, root_pid, MPI_COMM_WORLD);

    if (this_pid == root_pid) {
        printf("[TOTAL TIME] %.17g seconds.\n", max_total_time);
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}

RangeExtremes* computes_range_extremes(
    unsigned int total_number_of_processes,
    unsigned int total_numbers,
    unsigned int this_pid
) {
    RangeExtremes *range_extremes = malloc(sizeof(RangeExtremes));

    const int number_of_locations = total_numbers / total_number_of_processes;
    const int rest = total_numbers % total_number_of_processes;

    int start = this_pid * number_of_locations;
    int end = start + number_of_locations;

    int rest_offset_start = 0, rest_offset_end = 0;
    if (rest != 0) {
        if (this_pid == 0) {
            rest_offset_end = 1;
        } else if (this_pid <= rest) {
            rest_offset_start = this_pid;
            rest_offset_end = this_pid + ((this_pid < rest) ? 1 : 0);
        } else {
            rest_offset_start = rest_offset_end = rest;
        }
    }

    range_extremes->start = start + rest_offset_start;
    range_extremes->end = end + rest_offset_end;
    return range_extremes;
}
