#include "../include/preconditions.h"

void print_correct_usage(char *program_name) {
    printf("Correct usage: %s <strategy_id> <root_pid> <file_path_numbers>\n", 
        program_name == NULL ? "parallel-sum" : program_name);
}

void check_number_parameters(const unsigned int argc, char **argv) {
    if (argc != 4) {
        print_correct_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
}

void check_parameters(char **argv) {
    const char *strategy_id = argv[1];
    const char *root_pid = argv[2];
    const char *numbers_file = argv[3];
    if (!is_number(strategy_id)) {
        printf("The argument <strategy_id> must be a number!\n");
        exit(EXIT_FAILURE);
    } else if (!is_number(root_pid)) {
        printf("The argument <root_pid> must be a number!\n");
        exit(EXIT_FAILURE);
    } else if (access(numbers_file, F_OK) != 0) {
        printf("The argument <file_path_numbers> must be file that exists and contains numbers!\n");
        exit(EXIT_FAILURE);
    }
}

void check_strategy_id(const int strategy_id, const int total_number_of_processes, char *program_name) {
    if (strategy_id < 0 || strategy_id > 3) {
        printf("There is no strategy with id %d!\n", strategy_id);
        print_correct_usage(program_name);
        exit(EXIT_FAILURE);
    } else if (strategy_id != 1 && !is_power_of_two(total_number_of_processes)) {
        printf("Strategy %d can only be applied to a number of processes which "\
            "is a power of two!\n", strategy_id);
        exit(EXIT_FAILURE);
    }
}

bool is_power_of_two(const unsigned int number) {
    int log = log2(number);
    int poww = pow(2, log);
    return number == poww;
}

void check_root_pid(const int root_pid, const int total_number_of_processes) {
    if (root_pid != -1 && (root_pid < 0 || root_pid >= total_number_of_processes)) {
        printf("The argument root_pid provided does not identify any process!\n");
        exit(EXIT_FAILURE);
    }
}

void check_total_number_of_processes_less_than_or_equal_to_the_total_numbers(
    const int total_number_of_processes, 
    const int total_numbers
) {
    if (total_number_of_processes > total_numbers) {
        printf("The total number of processes can't be greater than N!\n");
        exit(EXIT_FAILURE);
    }
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