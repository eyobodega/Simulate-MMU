#Memory Management Unit (MMU) Visualizer#

Welcome to my MMU visualizer project! This program visualizes the working of a simple memory management unit. It reads logical addresses and translates them to their corresponding physical addresses. You can follow along the translation process and observe how the TLB and Page Table operate under different memory conditions.

Overview

The project contains two phases:

Phase 1 emulates the address translation and handling of page faults, assuming physical memory is the same size as the virtual address space (65,536 bytes).
Phase 2 goes a step further, simulating a smaller physical address space with 128 page frames rather than 256. It requires a page-replacement strategy to handle page faults when there is no free memory.
Setup and Usage

To get started, make sure you have the following files in your project directory:

Makefile
All relevant C source or header files
BACKING_STORE.bin
addresses.txt
test.sh
correct128.csv
correct256.csv
Run make in your terminal to build the project. Then, execute the following commands to start the visualizer:

Phase 1: ./mmu 256 BACKING_STORE.bin addresses.txt
Phase 2: ./mmu 128 BACKING_STORE.bin addresses.txt

Output

The program will output a CSV file (output256.csv for Phase 1, output128.csv for Phase 2) that includes:

The logical address being translated.
The corresponding physical address.
The signed byte value stored in physical memory at the translated physical address.
Additionally, the program will report the page-fault rate and the TLB hit rate.

Testing

You can use the provided test.sh script to test your project against the correct output files (correct256.csv for Phase 1, correct128.csv for Phase 2).
