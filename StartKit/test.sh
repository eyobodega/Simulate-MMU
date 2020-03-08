#!/bin/bash -e
echo "Compiling"
gcc mmu.c -o mmu
echo "Running"
./mmu BACKING_STORE.bin addresses.txt > out.txt
echo "Comparing"
diff out.txt correct.txt
