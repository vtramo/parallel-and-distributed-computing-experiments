#!/bin/bash

MPICC="mpicc"

BRed='\033[1;31m'
BWhite='\033[1;37m'
NC='\033[0m'

TOT_ITERATIONS=10
MAX_RANDOM_NUMBER=500
FILENAME_MATRIX_VALUES="matrix_values.txt"
FILENAME_RESULTS="results.txt"
IS_NUMBER_REGEX='^-?[0-9]+$'

numbers=()
square_matrix_dim=0
result_bmr=""
result_mxm=""
time_bmr=0
time_mxm=0

function read_square_matrix_dim() {
    input=""
    while [[ ! $input =~ $IS_NUMBER_REGEX || $input -le 0 || $(expr $input % ${PROCESSES}) -ne 0 ]]
    do
        printf "Enter the dimension of the square matrix: "
        read input

        if [[ ! $input =~ $IS_NUMBER_REGEX || $input -le 0 || $(expr $input % ${PROCESSES}) -ne 0 ]]
        then
            printf "${BRed}Incorrect dimension$NC\n"
        fi

        square_matrix_dim=$input
    done
}

function choose_printing_mode() {
    input=""

    declare -A boolean_values
    boolean_values[Y]="true"
    boolean_values[N]="false"

    while [[ $input != "Y" && $input != "N" ]]; do
        printf "Do you want print the matrix A, B and all local results on the stdout? (Y/N): "
        read input

        if [[ $input != "Y" && $input != "N" ]]; then
            printf "${BRed}Answer with Y or N!$NC\n"
        fi

        source .env
        export PRINT_MODE="${boolean_values[$input]}"
    done
}

function execute_bmr() {
    $MPICC -o bmr ./broadcast_multiply_rolling.c

    for ((i = 0; i < $TOT_ITERATIONS; i++)); do
        result_bmr=$(mpirun --oversubscribe -np $PROCESSES ./bmr $GRID_COMM_DIM $square_matrix_dim)
        this_time=$(echo "$result_bmr" | grep "TIME" | awk '{print $4}')
        time_bmr=$(awk -v a="$this_time" -v b="$time_bmr" 'BEGIN{print (a + b)}')
    done

    time_bmr=$(awk -v a="$time_bmr" -v b="$TOT_ITERATIONS" 'BEGIN{print (a / b)}')
    echo "$result_bmr"
    rm bmr
}

function execute_mxm() {
    gcc -o mxm ./single_process_matrix_per_matrix_product.c

    for ((i = 0; i < $TOT_ITERATIONS; i++)); do
        result_mxm=$(./mxm $square_matrix_dim)
        this_time=$(echo "$result_mxm" | grep "TIME" | awk '{print $5}')
        time_mxm=$(awk -v a="$this_time" -v b="$time_mxm" 'BEGIN{print (a + b)}')
    done

    time_mxm=$(awk -v a="$time_mxm" -v b="$TOT_ITERATIONS" 'BEGIN{print (a / b)}')
    echo "$result_mxm"
    rm mxm
}

echo "The number of processes is set equal to $PROCESSES."
echo "The grid of processes has size ${GRID_COMM_DIM}x${GRID_COMM_DIM}."
echo -e "You can change these settings by changing the env variables in the .env file.\n"

read_square_matrix_dim
choose_printing_mode
execute_bmr
execute_mxm

if [[ ! -e "$FILENAME_RESULTS" ]]; then
    printf "PARALLEL_TIME\tSINGLEP_TIME\tSPEED_UP\tTOTAL_PROCESSES\tSQUARE DIM\tGRID_COMM_DIMS\tTOT_NUMBERS\tTOT_ITERATIONS" > $FILENAME_RESULTS
fi

speed_up=$(awk -v a="$time_mxm" -v b="$time_bmr" 'BEGIN{print (a / b)}')
printf "\n$time_bmr\t$time_mxm\t$speed_up\t$PROCESSES\t$square_matrix_dim\t${GRID_COMM_DIM}x${GRID_COMM_DIM}\t$(($square_matrix_dim * $square_matrix_dim))\t$TOT_ITERATIONS" >> $FILENAME_RESULTS