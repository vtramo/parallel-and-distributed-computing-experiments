#!/bin/bash

# ----------------------------------------------------------------------------------

# COMPILATION 

mpicc ../src/main.c ../src/preconditions.c ../src/parallelsum.c -o ../build/main -lm

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

# --------------------------------------------------------------------------------------

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

function computesStrategyOne() {
    mpirun -np $1 ../build/main $2 $3 $4 | sort | uniq > $OUTPUT_FILENAME
    diff $OUTPUT_FILENAME $EXCEPTED_OUTPUT_FILENAME > /dev/null
    exitCode=$?
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

# ------------------------------------------------------------------------------------


#########
#TESTING#
#########


# TESTING STRATEGY ONE

echo "######################"
printf "#${UYellow}TESTING STRATEGY ONE${NC}#\n"
echo "######################"

# -----------------------------------



# TEST NUMBER 1 

echo -e \
"[PID 0] Result: 20\n\
[PID 1] Result: 5\n\
[PID 2] Result: 5\n\
[PID 3] Result: 5" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="1"
ROOT_PID="-1"
NUMBERS="5 5 5 5"
TEST_ID="1"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When perform the sum with 4 processes with root PID\n\
equals to -1, then the process with PID 0 should\n\
print the total sum whereas all the others should\n\
print partial sums."

computesStrategyOne "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"

printTestResult

deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------



# TEST NUMBER 2

echo "[PID 1] Result: 20" > $EXCEPTED_OUTPUT_FILENAME
  
NCPU="4"
STRATEGY="1"
ROOT_PID="1"
NUMBERS="5 5 5 5"
TEST_ID="2"

printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When perform the sum with 4 processes with PID 1 as root,\n\
then the process with PID 1 should print the correct result."

computesStrategyOne "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"

printTestResult

deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------



# TEST NUMBER 3

echo -e \
"Correct usage: ./../build/main <strategy_id> <root_pid> <numbers+>\n\
There is no strategy with id -2!" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="-2"
ROOT_PID="1"
NUMBERS="5 5 5 5"
TEST_ID="3"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When passing an invalid id strategy should print\n\
an error on stdout."

computesStrategyOne "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"

printTestResult

deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------



# TEST NUMBER 4

echo -e \
"Correct usage: ./../build/main <strategy_id> <root_pid> \
<numbers+>" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="1"
ROOT_PID="1"
NUMBERS=""
TEST_ID="4"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When passing less than 4 parameters, should print\n\
on stdout an error."

computesStrategyOne "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"

printTestResult

deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------



# TEST NUMBER 5

echo -e \
"Arguments must be integer numbers!
Correct usage: ./../build/main <strategy_id> <root_pid> \
<numbers+>" > $EXCEPTED_OUTPUT_FILENAME
  
NCPU="4"
STRATEGY="1"
ROOT_PID="1"
NUMBERS="hi!"
TEST_ID="5"

printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When passing a non-number argument should print an error."

computesStrategyOne "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"

printTestResult

deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------



# TEST NUMBER 6

echo -e \
"The total number of processes can't be greater than N!" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="1"
ROOT_PID="1"
NUMBERS="3 1 2"
TEST_ID="6"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When passing a total number of processes equal to 4 and a\n\
a total number of numbers less than 4 should\n\
print an error on the stdout."

computesStrategyOne "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"

printTestResult

deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------



# TEST NUMBER 7

echo -e \
"The argument root_pid provided does not identify any \
process!" > $EXCEPTED_OUTPUT_FILENAME
  
NCPU="4"
STRATEGY="1"
ROOT_PID="5"
NUMBERS="3 1 2"
TEST_ID="7"

printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When passing an invalid PID root should print\n\
an error on the stdout."

computesStrategyOne "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"

printTestResult

deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------

printSummary

# ----------------------------------------------------------------------------------