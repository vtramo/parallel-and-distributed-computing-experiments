#!/bin/bash

BRed='\033[1;31m'
BWhite='\033[1;37m'
NC='\033[0m'

MAX_RANDOM_NUMBER=500
IS_NUMBER_REGEX='^-?[0-9]+$'
FILENAME_VECTOR_VALUES="vector_values.txt"
FILENAME_MATRIX_VALUES="matrix_values.txt"

numbers=()
rows=0
columns=0
printing_mode="false"
total_threads=2

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
    gcc -o random_integer_generator random_integer_generator.c -std=c99
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

        total_threads=$input
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

function execute_pbs() {
    printf "\n${BWhite}EXECUTING PARALLEL MATRIX VECTOR PRODUCT$NC\n"
    qsub -v total_threads="$total_threads",rows="$rows",columns="$columns",printing_mode="$printing_mode" ./parallel_vector_matrix_product.pbs
    printf "\n${BWhite}Il pbs parallel_vector_matrix_product.pbs e' stato accodato. I risultati saranno stampati in parallel_vector_matrix_product.out e parallel_vector_matrix_product.err.$NC\n"
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

execute_pbs