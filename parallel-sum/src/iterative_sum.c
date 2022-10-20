#include <time.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    unsigned int size;
    int* numbers;
} Numbers;

Numbers* read_numbers_from_file(char *file_name);

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Correct usage: %s <file_path_numbers>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *file_name = argv[1];
    Numbers *bean_numbers = read_numbers_from_file(file_name);
    const int total_numbers = bean_numbers->size;
    const int *numbers = bean_numbers->numbers;

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