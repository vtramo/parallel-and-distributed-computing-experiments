#!/bin/bash

N_CPU=4
MAIN_PROGRAM="../../build/main"
MPI_MPICC_COMMAND="mpicc ../../src/main.c ../../src/preconditions.c ../../src/parallelsum.c -o ../../build/main -lm"
MPI_EXEC_COMMAND="mpirun -np $N_CPU"

TOTAL_RANDOM_NUMBERS=850
MAX_RANDOM_NUMBER=5000
RANDOM_INTEGER_GENERATOR_PROGRAM="../../build/random $TOTAL_RANDOM_NUMBERS $MAX_RANDOM_NUMBER"

DATE=$(date)
PERFORMANCE_OUTPUT_FILE="performance.txt"

TOTAL_ITERATIONS_TEST=250

STRATEGY_ONE=1
STRATEGY_TWO=2
STRATEGY_THREE=3
PID_ROOT=0

mean=0
function performance_testing() {
    STRATEGY_UNDER_TESTING=$1
    CalculatedTimes=()
    for ((i = 0; i < $TOTAL_ITERATIONS_TEST; i++)); do
        randomNumbers=$($RANDOM_INTEGER_GENERATOR_PROGRAM)
        output=$($MPI_EXEC_COMMAND $MAIN_PROGRAM $STRATEGY_UNDER_TESTING $PID_ROOT $randomNumbers)
        time=$(echo "$output" | grep "^\[TOTAL TIME\]" | awk '{ print $3 }')
        CalculatedTimes+=("$time")
    done
    echo ${CalculatedTimes[@]}
    computesAverageTime ${CalculatedTimes[@]}
    echo "[STRATEGY $STRATEGY_UNDER_TESTING] Average Time $mean seconds." >> "$PERFORMANCE_OUTPUT_FILE"
}

function computesAverageTime() {
    sum=0.00000
    for n in $@; do
        sum=$(echo "$sum + $n" | awk '{print $1 + $3}')
        echo $sum
    done
    mean=$(echo "$sum / $TOTAL_ITERATIONS_TEST" | awk '{print $1 / $3}')
}

$MPI_MPICC_COMMAND
rm "$PERFORMANCE_OUTPUT_FILE"
performance_testing $STRATEGY_ONE
performance_testing $STRATEGY_TWO
performance_testing $STRATEGY_THREE
