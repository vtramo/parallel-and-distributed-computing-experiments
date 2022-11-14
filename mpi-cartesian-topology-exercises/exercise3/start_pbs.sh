#!/bin/bash

if [ "$#" -ne 4 ]; then
  echo "Correct usage: $0 <processes> <grid_comm_rows> <matrix_rows> <matrix_columns>"
  exit 1
fi

N_CPU=$1
GRID_COMM_ROWS=$2
MATRIX_ROWS=$3
MATRIX_COLUMNS=$4

qsub -v N_CPU="$N_CPU",GRID_COMM_ROWS="$GRID_COMM_ROWS",MATRIX_ROWS="$MATRIX_ROWS",MATRIX_COLUMNS="$MATRIX_COLUMNS" ./matrix_partitioning_blocks.pbs