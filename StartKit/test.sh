#!/bin/bash -e
echo "Compiling"
gcc -w -o mmu mmu.c
echo "Running"
./mmu
#./mmu BACKING_STORE.bin addresses.txt > out.txt
#echo "Comparing"
#diff out.txt correct.txt