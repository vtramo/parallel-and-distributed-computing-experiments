#!/bin/bash

if [ "$#" -ne 4 ]; then
  echo "Correct usage: $0 <processes> <grid_comm_rows> <matrix_rows> <matrix_columns>"
  exit 1
fi

N_CPU=$1
COMM_GRID_ROWS=$2
ROWS=$3
COLUMNS=$4

qsub -v N_CPU="$N_CPU",COMM_GRID_ROWS="$COMM_GRID_ROWS",ROWS="$ROWS",COLUMNS="$COLUMNS" ./parallel_vector_matrix_product.pbs