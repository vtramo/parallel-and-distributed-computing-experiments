#!/bin/bash

N_CPU=4
MAIN_PROGRAM="../../build/main"
MPI_MPICC_COMMAND="mpicc ../../src/main.c ../../src/preconditions.c ../../src/parallel_sum.c -o ../../build/main -lm"
MPI_EXEC_COMMAND="mpirun -np $N_CPU"

TOTAL_RANDOM_NUMBERS="1000000"
MAX_RANDOM_NUMBER=5000
RANDOM_INTEGER_GENERATOR_PROGRAM="../../build/random $TOTAL_RANDOM_NUMBERS $MAX_RANDOM_NUMBER"

ITERATIVE_SUM_COMPILATION="gcc ../../src/iterative_sum.c -o ../../build/iterativesum"
ITERATIVE_SUM_PROGRAM="../../build/iterativesum"

DATE=$(date)
PERFORMANCE_OUTPUT_FILE="performance.txt"

NUMBERS_FILE_NAME="numbers"

TOTAL_ITERATIONS_TEST=10

STRATEGY_ONE=1
STRATEGY_TWO=2
STRATEGY_THREE=3
PID_ROOT=0

BWhite='\033[1;37m'
UGreen='\033[4;32m'
NC='\033[0m'

mean=0
function performance_testing_parallel_sum() {
    STRATEGY_UNDER_TESTING=$1
    printf "${UGreen}[INFO]$BWhite Testing the performance of strategy $STRATEGY_UNDER_TESTING...$NC\n"
    CalculatedTimes=()
    for ((i = 0; i < $TOTAL_ITERATIONS_TEST; i++)); do
        $RANDOM_INTEGER_GENERATOR_PROGRAM > $NUMBERS_FILE_NAME
        output=$($MPI_EXEC_COMMAND $MAIN_PROGRAM $STRATEGY_UNDER_TESTING $PID_ROOT $NUMBERS_FILE_NAME)
        time=$(echo "$output" | grep "^\[TOTAL TIME\]" | awk '{ print $3 }')
        CalculatedTimes+=("$time")
    done
    computesAverageTime ${CalculatedTimes[@]}
    echo "[STRATEGY $STRATEGY_UNDER_TESTING] Average Time $mean seconds." >> "$PERFORMANCE_OUTPUT_FILE"
    printf "${UGreen}[INFO]$BWhite Strategy $STRATEGY_UNDER_TESTING performance test completed$NC\n"
}

function performance_testing_iterative_sum() {
    printf "${UGreen}[INFO]$BWhite Testing the performance of iterative sum...$NC\n"
    CalculatedTimes=()
    for ((i = 0; i < $TOTAL_ITERATIONS_TEST; i++)); do
        $RANDOM_INTEGER_GENERATOR_PROGRAM > $NUMBERS_FILE_NAME
        output=$($ITERATIVE_SUM_PROGRAM $NUMBERS_FILE_NAME)
        time=$(echo "$output" | grep "^\[TOTAL TIME\]" | awk '{ print $3 }')
        CalculatedTimes+=("$time")
    done
    computesAverageTime ${CalculatedTimes[@]}
    echo "[ITERATIVE SUM] Average Time $mean seconds." >> "$PERFORMANCE_OUTPUT_FILE"
    printf "${UGreen}[INFO]$BWhite Iterative sum performance test completed$NC\n"
}

function computesAverageTime() {
    sum=0
    for n in $@; do
        sum=$(echo "$sum + $n" | awk '{print $1 + $3}')
    done
    mean=$(echo "$sum / $TOTAL_ITERATIONS_TEST" | awk '{print $1 / $3}')
}

$MPI_MPICC_COMMAND

if [[ -e "$PERFORMANCE_OUTPUT_FILE" ]]; then
    rm "$PERFORMANCE_OUTPUT_FILE"
fi

printf "$DATE\n\n" >> "$PERFORMANCE_OUTPUT_FILE"
performance_testing_parallel_sum $STRATEGY_ONE
performance_testing_parallel_sum $STRATEGY_TWO
performance_testing_parallel_sum $STRATEGY_THREE

$ITERATIVE_SUM_COMPILATION
performance_testing_iterative_sum
printf "${UGreen}[INFO]$BWhite Performance test completed!$NC\n"

rm $NUMBERS_FILE_NAME