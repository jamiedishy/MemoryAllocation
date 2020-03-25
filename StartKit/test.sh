#!/bin/bash -e
echo "Compiling"
gcc -w -o mmu mmu.c
echo "Running"
./mmu 
echo "Comparing"
diff output.csv correct.txt
