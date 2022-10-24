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
echo "5 5 5 5" > $NUMBERS_FILE_NAME
TEST_ID="1"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$(cat $NUMBERS_FILE_NAME)" "$TEST_ID" \
"When perform the sum with 4 processes with root PID\n\
equals to -1, then the process with PID 0 should\n\
print the corrected total sum whereas all the others should\n\
print corrected partial sums."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
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
echo "5 5 5 5" > $NUMBERS_FILE_NAME
TEST_ID="2"

printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$(cat $NUMBERS_FILE_NAME)" "$TEST_ID" \
"When perform the sum with 4 processes with PID 1 as root,\n\
then the process with PID 1 should print the correct result."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------

# TEST NUMBER 3

echo -e \
"Correct usage: ../build/main <strategy_id> <root_pid> <file_path_numbers>\n\
There is no strategy with id -2!" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="-2"
ROOT_PID="1"
echo "5 5 5 5" > $NUMBERS_FILE_NAME
TEST_ID="3"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$(cat $NUMBERS_FILE_NAME)" "$TEST_ID" \
"When passing an invalid id strategy should print\n\
an error on stdout."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------

# TEST NUMBER 4

echo -e \
"Correct usage: ../build/main <strategy_id> <root_pid> \
<file_path_numbers>" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="1"
ROOT_PID=""
echo "" > $NUMBERS_FILE_NAME
TEST_ID="4"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$(cat $NUMBERS_FILE_NAME)" "$TEST_ID" \
"When passing less than 4 parameters, should print\n\
on stdout an error."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------

# TEST NUMBER 5

echo -e "The total number of processes can't be greater than N!" > $EXCEPTED_OUTPUT_FILENAME
  
NCPU="4"
STRATEGY="1"
ROOT_PID="1"
echo "hi" > $NUMBERS_FILE_NAME
TEST_ID="5"

printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$(cat $NUMBERS_FILE_NAME)" "$TEST_ID" \
"When passing a non-number argument should print an error."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
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
echo "3 1 2" > $NUMBERS_FILE_NAME
TEST_ID="6"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$(cat $NUMBERS_FILE_NAME)" "$TEST_ID" \
"When passing a total number of processes equal to 4 and a\n\
a total number of numbers less than 4 should\n\
print an error on the stdout."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
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
echo "3 1 2" > $NUMBERS_FILE_NAME
TEST_ID="7"

printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$(cat $NUMBERS_FILE_NAME)" "$TEST_ID" \
"When passing an invalid PID root should print\n\
an error on the stdout."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
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
echo "1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1" > $NUMBERS_FILE_NAME
TEST_ID="8"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$(cat $NUMBERS_FILE_NAME)" "$TEST_ID" \
"When perform the sum with 4 processes with root PID\n\
equals to -1 with 40 numbers, then the process with PID 0 should\n\
print the correct total sum whereas all the others should\n\
print correct partial sums."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------

# TEST NUMBER 9

echo -e \
"[PID 0] Result: -1195\n\
[PID 1] Result: -1642\n\
[PID 2] Result: -802\n\
[PID 3] Result: -518" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="1"
ROOT_PID="-1"
echo "319 417 491 387 215 -62 -310 -282 -472 -190 -388 -340 291 81 -354 -480 321 -440 -440 357 -316" > $NUMBERS_FILE_NAME
TEST_ID="9"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$(cat $NUMBERS_FILE_NAME)" "$TEST_ID" \
"When perform the sum with 4 processes with root PID\n\
equals to -1 with 21 numbers, then the process with PID 0 should\n\
print the correct total sum whereas all the others should\n\
print correct partial sums."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------


# TEST NUMBER 10

echo -e "[PID 2] Result: 184" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="1"
ROOT_PID="2"
echo "319 417 491 387 215 -62 -310 -282 -472 -190 -388 -340 291 81 -354 -480 321 -440 -440 357 -316 35 87 100 -100 800 -213 -873 -222 -666 -101 234 2222 41 35" > $NUMBERS_FILE_NAME
TEST_ID="10"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$(cat $NUMBERS_FILE_NAME)" "$TEST_ID" \
"When perform the sum with 4 processes with root PID\n\
equals to 2 with 35 numbers, then the process with PID 2 should\n\
print the correct total sum."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -------------------------------

# TEST NUMBER 11

NCPU="4"
STRATEGY="1"
ROOT_PID="2"
MAX_VALUE_GENERATED=500
TOTAL_RANDOM_NUMBERS=5000
../build/random $TOTAL_RANDOM_NUMBERS $MAX_VALUE_GENERATED > $NUMBERS_FILE_NAME
TEST_ID="11"

sum "$(cat $NUMBERS_FILE_NAME)"
echo -e "[PID 2] Result: $EXPECTED_SUM" > $EXCEPTED_OUTPUT_FILENAME 

printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "Random numbers in '$NUMBERS_FILE_NAME'" "$TEST_ID" \
"When perform the sum with 4 processes with root PID\n\
equals to 2 with many random numbers, then the process with PID 2 should\n\
print the correct total sum."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS_FILE_NAME"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

printSummary

# ----------------------------------------------------------------------------------