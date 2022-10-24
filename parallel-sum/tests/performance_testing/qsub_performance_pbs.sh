#!/bin/bash

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <TOT_RANDOM_NUMBERS>"
	exit 1
fi

TOT_RANDOM_NUMBERS=$1
for ((n_cpu=2; n_cpu <= 2; n_cpu++)); do
	qsub -v N_CPU="$n_cpu",TOT_RANDOM_NUMBERS="$TOT_RANDOM_NUMBERS",TOT_ITERATIONS="5" ./performance_test.pbs
done
