#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

void print_correct_usage(char *program_name);
void check_number_parameters(const unsigned int argc, char **argv);
void check_the_parameters_are_all_numbers(const unsigned int argc, char **argv);
void check_strategy_id(const int strategy_id, char **argv);
void check_root_pid(const int root_pid, const int total_number_of_processes);
void check_total_number_of_processes_less_than_or_equal_to_the_total_numbers(
    const int total_number_of_processes, 
    const int total_numbers
);
bool isNumber(const char *s);
