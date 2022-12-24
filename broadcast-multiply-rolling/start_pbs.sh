#!/bin/bash

source .env
MPICC="mpicc"

BRed='\033[1;31m'
BWhite='\033[1;37m'
NC='\033[0m'

IS_NUMBER_REGEX='^-?[0-9]+$'

square_matrix_dim=0

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

    while [[ $input != "Y" && $input != "N" ]]; do
        printf "Do you want print the matrix A, B and all local results on the stdout? (Y/N): "
        read input

        if [[ $input != "Y" && $input != "N" ]]; then
            printf "${BRed}Answer with Y or N!$NC\n"
        else
            if [[ $input == "Y" ]]; then
                value="true"
            else
                value="false"
            fi
            echo "$(cat .env | sed "s/${PRINT_MODE}/${value}/")" > .env
        fi
    done
}

function execute_pbs() {
    printf "\n${BWhite}EXECUTING BROADCAST PROCESS MATRIX$NC\n"
    qsub -v square_matrix_dim="$square_matrix_dim",PROCESSES="$PROCESSES",GRID_COMM_DIM="$GRID_COMM_DIM",PRINT_MODE="$PRINT_MODE" ./broadcast_multiply_rolling.pbs
    printf "\n${BWhite}Il pbs broadcast_multiply_rolling.pbs e' stato accodato.\n"
    printf "I risultati saranno stampati in broadcast_multiply_rolling.pbs.out e broadcast_multiply_rolling.pbs.err.\n"
    printf "I risultati sulle performance verranno memorizzatin in results.txt.$NC\n"
}


echo "The number of processes is set equal to $PROCESSES."
echo "The grid of processes has size ${GRID_COMM_DIM}x${GRID_COMM_DIM}."
echo -e "You can change these settings by changing the env variables in the .env file.\n"

read_square_matrix_dim
choose_printing_mode
execute_pbs