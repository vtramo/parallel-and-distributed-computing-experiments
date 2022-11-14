#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Correct usage: $0 <processes> <grid_comm_rows>"
  exit 1
fi

N_CPU=$1
COMM_GRID_ROWS=$2

qsub -v N_CPU="$N_CPU",COMM_GRID_ROWS="$COMM_GRID_ROWS" ./grid_printer.pbs