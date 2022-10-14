#include "../include/preconditions.h"

void print_correct_usage(char *program_name) {
    printf("Correct usage: ./%s <strategy_id> <root_pid> <numbers+>\n", 
        program_name == NULL ? "sum-project" : program_name);
}

void check_number_parameters(const unsigned int argc, char **argv) {
    if (argc < 4) {
        printf("Correct usage: ./%s <strategy_id> <root_pid> <numbers+>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

void check_the_parameters_are_all_numbers(const unsigned int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (!isNumber(argv[i])) {
            printf("Arguments must be integer numbers!\n");
            printf("Correct usage: ./%s <strategy_id> <root_pid> <numbers+>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

void check_strategy_id(const int strategy_id, char **argv) {
    if (strategy_id < 0 || strategy_id > 3) {
        printf("There is no strategy with id %d!\n", strategy_id);
        printf("Correct usage: ./%s <strategy_id> <root_pid> <numbers+>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

void check_root_pid(const int root_pid, const int total_number_of_processes) {
    if (root_pid != -1 && (root_pid < 0 || root_pid >= total_number_of_processes)) {
        printf("The argument root_pid provided does not identify any process!\n");
        exit(EXIT_FAILURE);
    }
}

void check_total_number_of_processes_greater_than_or_equal_to_the_total_numbers(
    const int total_number_of_processes, 
    const int total_numbers
) {
    if (total_number_of_processes > total_numbers) {
        printf("The total number of processes can't be greater than N!\n");
        exit(EXIT_FAILURE);
    }
}

bool isNumber(const char *s) {
    if (s == NULL) return false;
    for (int i = 0; s[i] != '\0'; i++) {
        if (i == 0 && s[i] == '-' && isdigit(s[i + 1])) continue;
        if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}