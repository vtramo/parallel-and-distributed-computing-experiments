#!/bin/bash

gcc -o mxm single_process_matrix_per_matrix_product.c
./mxm 128
rm mxm