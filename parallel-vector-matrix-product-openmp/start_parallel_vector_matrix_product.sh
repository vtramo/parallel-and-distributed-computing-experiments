#!/bin/bash

BRed='\033[1;31m'
BWhite='\033[1;37m'
NC='\033[0m'

TOT_ITERATIONS=5
MAX_RANDOM_NUMBER=500
FILENAME_VECTOR_VALUES="vector_values.txt"
FILENAME_MATRIX_VALUES="matrix_values.txt"
FILENAME_RESULTS="results.txt"
IS_NUMBER_REGEX='^-?[0-9]+$'

numbers=()
rows=0
columns=0
printing_mode="false"
result_parallel_vector_matrix_product=""
result_single_thread_vector_matrix_product=""
time_parallel_vector_matrix_product=0
time_single_thread_vector_matrix_product=0

function read_rows() {
    input=""
    while [[ ! $input =~ $IS_NUMBER_REGEX || $input -le 0 ]]
    do
        printf "Enter the number of rows: "
        read input

        if [[ ! $input =~ $IS_NUMBER_REGEX || $input -le 0 ]]
        then
            printf "${BRed}Incorrect number of rows$NC\n"
        fi

        rows=$input
    done
}

function read_columns() {
    input=""
    while [[ ! $input =~ $IS_NUMBER_REGEX || $input -le 0 ]]
    do
        printf "Enter the number of columns: "
        read input

        if [[ ! $input =~ $IS_NUMBER_REGEX || $input -le 0 ]]
        then
            printf "${BRed}Incorrect number of columns$NC\n"
        fi

        columns=$input
    done
}

function read_matrix_numbers() {
      input=""
      local local_numbers=()

      for ((i = 0; i < $rows; i++)); do
          for ((j = 0; j < $columns; j++)); do
              while [[ ! $input =~ $IS_NUMBER_REGEX ]]; do
                  printf "Position A($i, $j): "
                  read input

                  if [[ ! $input =~ $IS_NUMBER_REGEX ]]; then
                      printf "${BRed}The input must be a number!$NC\n"
                  else
                      local_numbers+=($input)
                  fi
              done
              input=""
          done
      done

      numbers=("${local_numbers[@]}")
}

function read_vector() {
    input=""
    local local_numbers=()

    for ((i = 0; i < $columns; i++)); do
        while [[ ! $input =~ $IS_NUMBER_REGEX ]]; do
            printf "Position x($i): "
            read input

            if [[ ! $input =~ $IS_NUMBER_REGEX ]]; then
                printf "${BRed}The input must be a number!$NC\n"
            else
                local_numbers+=($input)
            fi
        done
        input=""
    done

    numbers=("${local_numbers[@]}")
}

function generate_random_numbers() {
    total_random_numbers=$1
    gcc -o random_integer_generator random_integer_generator.c
    numbers=$(./random_integer_generator $total_random_numbers $MAX_RANDOM_NUMBER)
    printf "\n${BWhite}$total_random_numbers random numbers have just been generated\n"
    rm random_integer_generator
}

function read_number_of_threads() {
    input=" "
    while [[ ! $input =~ $IS_NUMBER_REGEX ]]
    do
        printf "Enter the number of threads: "
        read input

        if [[ ! $input =~ $IS_NUMBER_REGEX ]]
        then
            printf "${BRed}Incorrect number of threads$NC\n"
        fi

        export OMP_NUM_THREADS=$input
    done
}

function choose_printing_mode() {
    input=""

    declare -A boolean_values
    boolean_values[Y]="true"
    boolean_values[N]="false"

    while [[ $input != "Y" && $input != "N" ]]; do
        printf "Print matrix A, vector x and result b on stdout? (Y/N): "
        read input

        if [[ $input != "Y" && $input != "N" ]]; then
            printf "${BRed}Answer with Y or N!$NC\n"
        fi

        printing_mode="${boolean_values[$input]}"
    done
}

function execute_parallel_vector_matrix_product() {
    gcc -fopenmp -lgomp -o parallel_vector_matrix_product parallel_vector_matrix_product.c

    for ((i = 0; i < $TOT_ITERATIONS; i++)); do
        result_parallel_vector_matrix_product=$(./parallel_vector_matrix_product $rows $columns $FILENAME_MATRIX_VALUES $FILENAME_VECTOR_VALUES $printing_mode)
        this_time=$(echo "$result_parallel_vector_matrix_product" | grep "TIME" | awk '{print $4}')
        time_parallel_vector_matrix_product=$(awk -v a="$this_time" -v b="$time_parallel_vector_matrix_product" 'BEGIN{print (a + b)}')
    done

    time_parallel_vector_matrix_product=$(awk -v a="$time_parallel_vector_matrix_product" -v b="$TOT_ITERATIONS" 'BEGIN{print (a / b)}')
    echo "$result_parallel_vector_matrix_product"
    rm parallel_vector_matrix_product
}

function execute_single_thread_vector_matrix_product() {
    gcc -o single_thread_vector_matrix_product single_thread_vector_matrix_product.c

    for ((i = 0; i < $TOT_ITERATIONS; i++)); do
        result_single_thread_vector_matrix_product=$(./single_thread_vector_matrix_product $rows $columns $FILENAME_MATRIX_VALUES $FILENAME_VECTOR_VALUES)
        this_time=$(echo "$result_single_thread_vector_matrix_product" | grep "TIME" | awk '{print $5}')
        time_single_thread_vector_matrix_product=$(awk -v a="$this_time" -v b="$time_single_thread_vector_matrix_product" 'BEGIN{print (a + b)}')
    done

    time_single_thread_vector_matrix_product=$(awk -v a="$time_single_thread_vector_matrix_product" -v b="$TOT_ITERATIONS" 'BEGIN{print (a / b)}')
    echo "$result_single_thread_vector_matrix_product"
    rm single_thread_vector_matrix_product
}

read_rows
read_columns

TOTAL_NUMBERS=$(( $rows * $columns ))
if (( $TOTAL_NUMBERS > 20 )); then
    generate_random_numbers $TOTAL_NUMBERS # Matrix A
    echo "${numbers[@]}" > $FILENAME_MATRIX_VALUES
    generate_random_numbers $columns # Vector x
    echo "${numbers[@]}" > $FILENAME_VECTOR_VALUES
else
    read_matrix_numbers # Matrix A
    echo "${numbers[@]}" > $FILENAME_MATRIX_VALUES
    read_vector # Vector x
    echo "${numbers[@]}" > $FILENAME_VECTOR_VALUES
fi

read_number_of_threads
choose_printing_mode

execute_single_thread_vector_matrix_product
execute_parallel_vector_matrix_product

rm $FILENAME_VECTOR_VALUES
rm $FILENAME_MATRIX_VALUES

if [[ ! -e "$FILENAME_RESULTS" ]]; then
    printf "PARALLEL_TIME\tSINGLETH_TIME\tSPEED_UP_P/ST\tTOTAL_THREADS\tROWS\tCOLUMNS\tTOT_NUMBERS\tTOT_ITERATIONS" > $FILENAME_RESULTS
fi

speed_up=$(awk -v a="$time_single_thread_vector_matrix_product" -v b="$time_parallel_vector_matrix_product" 'BEGIN{print (a / b)}')
printf "\n$time_parallel_vector_matrix_product\t$time_single_thread_vector_matrix_product\t$speed_up\t$OMP_NUM_THREADS\t$rows\t$columns\t$(($rows * $columns))\t$TOT_ITERATIONS" >> $FILENAME_RESULTS