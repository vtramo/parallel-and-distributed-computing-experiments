#!/bin/bash

source ./test_global.sh

# TESTING STRATEGY TWO

echo "######################"
printf "#${UYellow}TESTING STRATEGY TWO${NC}#\n"
echo "######################"

# -----------------------------------

# TEST NUMBER 1

echo -e "Strategy 2 can only be applied to a number of processes \
which is a power of two!" > $EXCEPTED_OUTPUT_FILENAME

NCPU="3"
STRATEGY="2"
ROOT_PID="-1"
NUMBERS="5 5 5 5"
TEST_ID="1"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When perform the sum with 3 processes with strategy 2,\n\
then should print on stdout an error"

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------




# TEST NUMBER 2

echo -e \
"[PID 0] Result: 20\n\
[PID 1] Result: 5\n\
[PID 2] Result: 10\n\
[PID 3] Result: 5" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="2"
ROOT_PID="-1"
NUMBERS="5 5 5 5"
TEST_ID="2"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When perform the parallel sum with strategy 2 with root\n\
PID equals to -1, then the PID 0 should print the\n\
corrected total sum whereas all the others should\n\
print partial sums."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------


# TEST NUMBER 3

echo -e "[PID 1] Result: 5" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="2"
ROOT_PID="1"
NUMBERS="5 5 5 5"
TEST_ID="3"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When perform the parallel sum with strategy 2 with root\n\
PID equals to 1, then only the PID 1 should print the\n\
corrected partial sum."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------


# TEST NUMBER 4

echo -e "[PID 0] Result: 40" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="2"
ROOT_PID="0"
NUMBERS="1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1"
TEST_ID="4"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When perform the parallel sum with strategy 2 with root\n\
PID equals to 0 with 40 numbers, then only the PID 0 should print the\n\
corrected total sum."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------