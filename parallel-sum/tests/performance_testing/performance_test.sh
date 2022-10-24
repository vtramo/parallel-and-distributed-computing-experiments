#!/bin/bash

if [[ "$#" -ne 3 ]]; then
	echo "Usage: $0 <N_CPU> <TOTAL_RANDOM_NUMBERS> <TOTAL_ITERATIONS_TEST>"
	echo "N_CPU: $1"
	echo "TOTAL_RANDOM_NUMBERS: $2"
	echo "TOTAL_ITERATIONS_TEST: $3"
	exit 1
fi

N_CPU=$1
TOTAL_RANDOM_NUMBERS=$2
TOTAL_ITERATIONS_TEST=$3

MAIN_PROGRAM="$PBS_O_WORKDIR/build/main"
MPI_MPICC_COMMAND="/usr/lib64/openmpi/1.4-gcc/bin/mpicc $PBS_O_WORKDIR/src/main.c $PBS_O_WORKDIR/src/preconditions.c $PBS_O_WORKDIR/src/parallel_sum.c -o $PBS_O_WORKDIR/build/main -lm -std=c99"
MPI_EXEC_COMMAND="/usr/lib64/openmpi/1.4-gcc/bin/mpiexec -machinefile hostlist -n $N_CPU"
MPI_EXEC_COMMAND_ITERATIVE="/usr/lib64/openmpi/1.4-gcc/bin/mpiexec -n 1"
MAX_RANDOM_NUMBER=5000
RANDOM_INTEGER_GENERATOR_PROGRAM="$PBS_O_WORKDIR/build/random $TOTAL_RANDOM_NUMBERS $MAX_RANDOM_NUMBER"

ITERATIVE_SUM_COMPILATION="/usr/lib64/openmpi/1.4-gcc/bin/mpicc $PBS_O_WORKDIR/src/iterative_sum.c -o $PBS_O_WORKDIR/build/iterativesum -std=c99"
ITERATIVE_SUM_PROGRAM="$PBS_O_WORKDIR/build/iterativesum"

RANDOM_PROGRAM_COMPILATION="/usr/lib64/openmpi/1.4-gcc/bin/mpicc -o $PBS_O_WORKDIR/build/random $PBS_O_WORKDIR/src/random_integer_generator.c -std=c99"

DATE=$(date)
PERFORMANCE_OUTPUT_FILE="$PBS_O_WORKDIR/tests/performance_testing/performance_test_results.txt"

NUMBERS_FILE_NAME="numbers"


STRATEGY_ONE=1
STRATEGY_TWO=2
STRATEGY_THREE=3
PID_ROOT=0

BWhite='\033[1;37m'
UGreen='\033[4;32m'
NC='\033[0m'

mean=0
AVERAGE_TIME_STRATEGY_ONE=0
AVERAGE_TIME_STRATEGY_TWO=0
AVERAGE_TIME_STRATEGY_THREE=0
AVERAGE_TIME_ITERATIVE=0

function performance_testing_parallel_sum() {
    STRATEGY_UNDER_TESTING=$1
    printf "${UGreen}[INFO]$BWhite Testing the performance of strategy $STRATEGY_UNDER_TESTING...$NC\n"
    CalculatedTimes=()
    for ((i = 0; i < $TOTAL_ITERATIONS_TEST; i++)); do
        $RANDOM_INTEGER_GENERATOR_PROGRAM > $NUMBERS_FILE_NAME
		cat $NUMBERS_FILE_NAME | wc -w
        output=$($MPI_EXEC_COMMAND $MAIN_PROGRAM $STRATEGY_UNDER_TESTING $PID_ROOT $NUMBERS_FILE_NAME)
        time=$(echo "$output" | grep "^\[TOTAL TIME\]" | awk '{ print $3 }')
        CalculatedTimes+=("$time")
    done
    computesAverageTime ${CalculatedTimes[@]}
	if [[ $STRATEGY_UNDER_TESTING -eq 1 ]]; then
		AVERAGE_TIME_STRATEGY_ONE="$mean"
	elif [[ $STRATEGY_UNDER_TESTING -eq 2 ]]; then
		AVERAGE_TIME_STRATEGY_TWO="$mean"
	elif [[ $STRATEGY_UNDER_TESTING -eq 3 ]]; then
		AVERAGE_TIME_STRATEGY_THREE="$mean"
	fi
    printf "${UGreen}[INFO]$BWhite Strategy $STRATEGY_UNDER_TESTING performance test completed$NC\n"
}

function performance_testing_iterative_sum() {
    printf "${UGreen}[INFO]$BWhite Testing the performance of iterative sum...$NC\n"
    CalculatedTimes=()
    for ((i = 0; i < $TOTAL_ITERATIONS_TEST; i++)); do
        $RANDOM_INTEGER_GENERATOR_PROGRAM > $NUMBERS_FILE_NAME
        output=$($MPI_EXEC_PROGRAM_ITERATIVE $ITERATIVE_SUM_PROGRAM $NUMBERS_FILE_NAME)
        time=$(echo "$output" | grep "^\[TOTAL TIME\]" | awk '{print $3}')
        CalculatedTimes+=("$time")
    done
    computesAverageTime ${CalculatedTimes[@]}
	AVERAGE_TIME_ITERATIVE="$mean"
    printf "${UGreen}[INFO]$BWhite Iterative sum performance test completed$NC\n"
}

function print_testing_info() {
	echo "$STRATEGY_ONE $N_CPU $TOTAL_ITERATIONS_TEST $TOTAL_RANDOM_NUMBERS $AVERAGE_TIME_STRATEGY_ONE $AVERAGE_TIME_ITERATIVE" >> "$PERFORMANCE_OUTPUT_FILE"
	if [[ $N_CPU -eq 2 || $N_CPU -eq 4 || $N_CPU -eq 8 ]]; then
		echo "$STRATEGY_TWO $N_CPU $TOTAL_ITERATIONS_TEST $TOTAL_RANDOM_NUMBERS $AVERAGE_TIME_STRATEGY_TWO $AVERAGE_TIME_ITERATIVE" >> "$PERFORMANCE_OUTPUT_FILE"
		echo "$STRATEGY_THREE $N_CPU $TOTAL_ITERATIONS_TEST $TOTAL_RANDOM_NUMBERS $AVERAGE_TIME_STRATEGY_THREE $AVERAGE_TIME_ITERATIVE" >> "$PERFORMANCE_OUTPUT_FILE"
	fi
}

function computesAverageTime() {
    sum=0.000000000000000000000
    for n in $@; do
        sum=$(echo "$sum + $n" | awk '{print $1 + $3}')
    done
    mean=$(echo "$sum / $TOTAL_ITERATIONS_TEST" | awk '{print $1 / $3}')
}

$MPI_MPICC_COMMAND
$RANDOM_PROGRAM_COMPILATION

performance_testing_parallel_sum $STRATEGY_ONE
if [[ $N_CPU -eq 2 || $N_CPU -eq 4 || $N_CPU -eq 8 ]]; then 
	performance_testing_parallel_sum $STRATEGY_TWO
	performance_testing_parallel_sum $STRATEGY_THREE
fi

$ITERATIVE_SUM_COMPILATION
performance_testing_iterative_sum
print_testing_info

printf "${UGreen}[INFO]$BWhite Performance test completed!$NC\n"
printf "${UGreen}[INFO]$BWhite I risultati sono stati registrati in $PERFORMANCE_OUTPUT_FILE.$NC\n"
rm $NUMBERS_FILE_NAME
