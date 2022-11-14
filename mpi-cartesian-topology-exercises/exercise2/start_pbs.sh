#!/bin/bash

if [ "$#" -ne 3 ]; then
  echo "Correct usage: $0 <processes> <total_random_numbers> <strategy>"
  exit 1
fi

N_CPU=$1
TOTAL_RANDOM_NUMBERS=$2
STRATEGY=$3

qsub -v N_CPU="$N_CPU",TOTAL_RANDOM_NUMBERS="$TOTAL_RANDOM_NUMBERS",STRATEGY="$STRATEGY" ./uniform_distributor_numbers.pbs