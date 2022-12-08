#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

void print_correct_usage(char *program_name);
int* generate_random_numbers(const unsigned int total, const unsigned int max);
bool is_number(const char *s);

int main(int argc, char **argv) {
    if (argc != 3) {
        print_correct_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!is_number(argv[1])) {
        printf("The arguments must be a positive number!\n");
        print_correct_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    const int total_random_numbers = atoi(argv[1]);
    const int max = atoi(argv[2]);
    if (total_random_numbers <= 0 || max <= 0) {
        printf("The arguments must be a positive number!\n");
        exit(EXIT_FAILURE);
    }

    int *random_numbers = generate_random_numbers(total_random_numbers, max);

    for (int i = 0; i < total_random_numbers; i++) 
        printf("%d%s", random_numbers[i], (i + 1 < total_random_numbers) ? " " : "");
    
    free(random_numbers);
    return EXIT_SUCCESS;
}

void print_correct_usage(char *program_name) {
    printf("Correct usage: %s <total_random_numbers> <max>\n", program_name);
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

