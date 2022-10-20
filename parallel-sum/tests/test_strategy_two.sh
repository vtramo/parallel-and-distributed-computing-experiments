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

# TEST NUMBER 5

echo -e \
"[PID 0] Result: -1195\n\
[PID 1] Result: -1642\n\
[PID 2] Result: -1320\n\
[PID 3] Result: -518" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="2"
ROOT_PID="-1"
NUMBERS="319 417 491 387 215 -62 -310 -282 -472 -190 -388 -340 291 81 -354 -480 321 -440 -440 357 -316"
TEST_ID="5"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When perform the sum with 4 processes with root PID\n\
equals to -1 with 21 numbers, then the process with PID 0 should\n\
print the correct total sum whereas all the others should\n\
print correct partial sums."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------


# TEST NUMBER 6

echo -e "[PID 2] Result: 980" > $EXCEPTED_OUTPUT_FILENAME

NCPU="4"
STRATEGY="2"
ROOT_PID="2"
NUMBERS="319 417 491 387 215 -62 -310 -282 -472 -190 -388 -340 291 81 -354 -480 321 -440 -440 357 -316 35 87 100 -100 800 -213 -873 -222 -666 -101 234 2222 41 35"
TEST_ID="6"
  
printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When perform the sum with 4 processes with root PID\n\
equals to 2 with 35 numbers, then the process with PID 2 should\n\
print the correct partial sum."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

# -----------------------------------

# TEST NUMBER 7

NCPU="4"
STRATEGY="2"
ROOT_PID="0"
TOTAL_RANDOM_NUMBERS=850
MAX_VALUE_GENERATED=999
NUMBERS=$(../build/random $TOTAL_RANDOM_NUMBERS $MAX_VALUE_GENERATED)
TEST_ID="7"

sum $NUMBERS
echo -e "[PID 0] Result: $EXPECTED_SUM" > $EXCEPTED_OUTPUT_FILENAME 

printTestHeader "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS" "$TEST_ID" \
"When perform the sum with 4 processes with root PID\n\
equals to 0 with many random numbers, then the process with PID 0 should\n\
print the correct total sum."

computesParallelSum "$NCPU" "$STRATEGY" "$ROOT_PID" "$NUMBERS"
compareOutputWithExpectedOutput
printTestResult
deletesGeneratedFiles

echo -e "$LINE\n"

printSummary

# ----------------------------------------------------------------------------------