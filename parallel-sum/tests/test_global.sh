#!/bin/bash

# ----------------------------------------------------------------------------------

# COMPILATION 

mpicc ../src/main.c ../src/preconditions.c ../src/parallel_sum.c -o ../build/main -lm

if [ $? -ne 0 ]; then
    exit 1
fi

# ----------------------------------------------------------------------------------

# [UI] COLORS & LINES

GREEN='\033[1;32m'
BRed='\033[1;31m'
UYellow='\033[4;33m' 
NC='\033[0m'
BWhite='\033[1;37m'
UWhite='\033[4;37m'
BPurple='\033[1;35m' 
LINE="-------------------------"

# ------------------------------------------------------------------------------------

# GLOBAL VARIABLES 

totalTests=0
EXCEPTED_OUTPUT_FILENAME="expectedOutput"
OUTPUT_FILENAME="output"
failures=0
exitCode=0

# -------------------------------------------------------------------------------------

# FUNCTIONS

function printTestHeader() {
    echo -e "$LINE\n"
    echo -e "${UWhite}| TEST #$5 |$NC\n"
    echo -e "${BPurple}Description:$NC $6\n"
    echo -e "${BWhite}INPUT$NC\n"
    echo "- Number of processes: $1"
    echo "- Strategy: $2"
    echo "- Root PID: $3"
    echo "- Numbers: $4"
    echo -e "\n${BWhite}EXPECTED OUTPUT$NC:\n"
    cat $EXCEPTED_OUTPUT_FILENAME
    totalTests=$((totalTests+1))
}

function computesParallelSum() {
    mpirun -np $1 ../build/main $2 $3 $4 | sort | uniq > $OUTPUT_FILENAME
}

function compareOutputWithExpectedOutput() {
    if grep -q "TOTAL TIME" $OUTPUT_FILENAME; then
        grep -v "TOTAL TIME" $OUTPUT_FILENAME > tmpfile && mv tmpfile $OUTPUT_FILENAME
        rm tmpfile
    fi
    diff $OUTPUT_FILENAME $EXCEPTED_OUTPUT_FILENAME > /dev/null
    exitCode=$?
}

function printTestResult() {
    if [ $exitCode -ne 0 ]; then
        printf "\n${BRed}BUT WAS:$NC\n\n"
        cat $OUTPUT_FILENAME
        printf "\n${BRed}TEST FAIL!$NC\n\n"
        failures=$((failures+1))
    else 
        printf "\n${GREEN}TEST OK!$NC\n\n"
    fi
}

function deletesGeneratedFiles() {
    rm $EXCEPTED_OUTPUT_FILENAME $OUTPUT_FILENAME
}

function printSummary() {
    printf "${UWhite}SUMMARY$NC\n\n"
    testPassed=$((totalTests - failures))
    printf "${BWhite}TOTAL TESTS: $totalTests$NC\n"
    printf "${GREEN}TEST PASSED: $testPassed$NC\n"
    printf "${BRed}FAILURES: $failures$NC\n\n"
    echo -e $LINE
}

# -----------------------------------------------------------------