#!/bin/bash

source ./test_global.sh

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
print the corrected total sum whereas all the others should\n\
print corrected partial sums."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
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

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
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

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
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

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
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

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
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

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
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

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------

# TEST NUMBER 8

echo -e \
"[PID 0] Result: 40\n\
[PID 1] Result: 10\n\
[PID 2] Result: 10\n\
[PID 3] Result: 10" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="1"
ROOT_PID="-1"
NUMBERS="1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1"
TEST_ID="8"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When perform the sum with 4 processes with root PID\n\
equals to -1 with 40 numbers, then the process with PID 0 should\n\
print the correct total sum whereas all the others should\n\
print correct partial sums."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------

printSummary

# ----------------------------------------------------------------------------------