#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

void print_correct_usage(char *program_name);
void check_number_parameters(const unsigned int argc, char **argv);
void check_parameters(char **argv);
void check_strategy_id(const int strategy_id, const int total_number_of_processes, char *program_name);
bool is_power_of_two(const unsigned int number);
void check_root_pid(const int root_pid, const int total_number_of_processes);
void check_total_number_of_processes_less_than_or_equal_to_the_total_numbers(
    const int total_number_of_processes, 
    const int total_numbers
);
bool is_number(const char *s);
