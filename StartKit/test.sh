#!/bin/bash -e
echo "Compiling"
gcc mmu.c -o mmu
echo "Running"
./mmu v1 BACKING_STORE.bin addresses.txt > out.txt
echo "Comparing"
diff out.txt correct.txt
