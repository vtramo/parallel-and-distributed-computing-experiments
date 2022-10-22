#!/bin/bash

mpicc ../src/main.c ../src/preconditions.c ../src/parallel_sum.c -o ../build/main -lm

BRed='\033[1;31m'
BWhite='\033[1;37m'
NC='\033[0m'

isNumber='^-?[0-9]+$'

STRATEGY_ONE=1
STRATEGY_TWO=2
STRATEGY_THREE=3

MIN_CPU=1
MAX_CPU=8

N_CPU=0
TOT_N=0
NUMBERS=()
STRATEGY=0
PID_ROOT=0

function readNumberOfCPU() {
    input=""
    while [[ ! $input =~ $isNumber || $input -lt $MIN_CPU || $input -gt $MAX_CPU ]]
    do
        printf "Enter the number of processors (min $MIN_CPU - max $MAX_CPU): "
        read input

        if [[ ! $input =~ $re || $input -lt $MIN_CPU || $input -gt $MAX_CPU ]]
        then
            printf "${BRed}Incorrect number of processors$NC\n"
        fi

        N_CPU=$input
    done 
}

function readTotalNumbers() {
    input=""
    while [[ ! $input =~ $isNumber || $input -lt $N_CPU ]]; do
        printf "Enter the number N of numbers to add: "
        read input

        if [[ ! $input =~ $isNumber ]]; then
            printf "${BRed}The input must be a positive number!$NC\n"
        elif [[ $input -lt $N_CPU ]]; then
            printf "${BRed}The total number of numbers to add cannot be less than the total number of processors!$NC\n"
        fi

        TOT_N=$input
    done
}

function readNumbers() {
    printf "\n"
    current=1
    Numbers=()
    while [[ $current -le $TOT_N ]]; do
        input=""
        while [[ ! $input =~ $isNumber ]]; do
            printf "Number $current: "
            read input

            if [[ ! $input =~ $isNumber ]]; then
                printf "${BRed}The input must be a number!$NC\n"
            else
                Numbers+=($input)
                current=$((current+1))
            fi
        done
    done
    NUMBERS=("${Numbers[@]}")
}

function generateRandomNumbers() {
    NUMBERS=$(../build/random $TOT_N 500)
    printf "\n${BWhite}$TOT_N random numbers have just been generated\n"
}

function printSummary() {
    Numbers=$@

    echo -e "\nTOTAL CPU: $N_CPU"
    echo "TOTAL NUMBERS: $TOT_N"

    sum=0
    for i in $Numbers; do
        sum=$((sum+$i))
    done

    echo -e "TOTAL SUM: $sum\n"
}

function readStrategy() {
    if isPowerOfTwo "$N_CPU"; then
        input=""
        while [[ ! $input =~ $isNumber || $input -lt $STRATEGY_ONE || $input -gt $STRATEGY_THREE ]]
        do
            printf "Enter the strategy to use (1, 2 or 3): "
            read input

            if [[ ! $input =~ $isNumber ]]; then
                printf "${BRed}The input must be a positive number!$NC\n"
            elif [[ $input -lt $STRATEGY_ONE || $input -gt $STRATEGY_THREE ]]; then
                printf "${BRed}This strategy doesn't exist!$NC\n"
            else
                STRATEGY=$input
            fi
        done
    else
        printf "${BWhite}With this setting you can only use strategy 1, so strategy 1 has been chosen automatically.$NC\n\n"
        STRATEGY=$STRATEGY_ONE
    fi
}

function isPowerOfTwo() {
    declare -i n=$1
    (( n > 0 && (n & (n - 1)) == 0 ))
}

function readPIDRoot() {
    MAX_PID=$((N_CPU - 1))
    input=""
    while [[ ! $input =~ $isNumber || ($input -lt 0 && $input -ne -1) || $input -ge $N_CPU ]]
    do
        printf "Enter the pid that must print the result (0 to ${MAX_PID} or -1 if everyone has to print the result): "
        read input

        if [[ ($input -ne -1 && ! $input =~ $isNumber) ]]; then
            printf "${BRed}The input must be a number!$NC\n"
        elif [[ ($input -lt 0 && $input -ne -1) || $input -gt $MAX_PID ]]; then
            printf "${BRed}This PID doesn't exist!$NC\n"
        else
            PID_ROOT=$input
        fi
    done
}

function executeParallelSum() {
    printf "\n${BWhite}EXECUTING PARALLEL SUM$NC\n"
    echo "$@" > numbers
    mpirun -np $N_CPU ../build/main $STRATEGY $PID_ROOT "numbers"
}

printf "${BWhite}PARALLEL SUM $NC\n\n"

readNumberOfCPU
readTotalNumbers

if [[ $TOT_N -le 20 ]]; then
    readNumbers
else
    generateRandomNumbers
fi

printSummary ${NUMBERS[@]}
readStrategy
readPIDRoot

executeParallelSum ${NUMBERS[@]}
rm numbers