#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Correct usage: %s <numbers+>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const int total_numbers = argc - 1;
    int *numbers = malloc(sizeof(int) * total_numbers);

    for (int i = 1; i <= total_numbers; i++) {
        numbers[i-1] = atoi(argv[i]);
    }

    clock_t t0 = clock();

    int sum = 0;
    for (int i = 0; i < total_numbers; i++) {
        sum += numbers[i];
    }

    clock_t t1 = clock();

    const double total_time = (double)(t1 - t0) / CLOCKS_PER_SEC;

    printf("[ITERATIVE SUM] Result: %d\n", sum);
    printf("[TOTAL TIME] %f seconds.\n", total_time);

    return EXIT_SUCCESS;
}