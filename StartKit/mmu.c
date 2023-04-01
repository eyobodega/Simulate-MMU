#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//function prototypes
void init_page_table();

//define sizes
#define PAGE_TABLE_SIZE 256

//define page entry with default value of valid to -1
typedef struct page_entry {
    unsigned char frame;
    int valid;
} page_table_entry;

//intial page table array as an array of structs page_table_entry
page_table_entry page_table[PAGE_TABLE_SIZE];

int main(int argc, char *argv[]) {

    //intialise page table entries to -1
    init_page_table();

    // TODO: Implement program logic here
    FILE *file_ptr;
    char buffer[100];

    //could change this to accept a file name as a command line argument
    file_ptr = fopen("addresses.txt", "r");

    // Read file contents into buffer, parse buffer to get page number and offset
    while (fgets(buffer, sizeof(buffer), file_ptr) != NULL){
        int address = atoi(buffer);
        int page = address >> 8;
        int offset = address & 0xFF;
        // printf("Virtual address %d: ", address);
        // printf("page number = %d, ", page);
        // printf("offset = %d\n", offset);
        //send address to page table
        


    }
    // Close file
    fclose(file_ptr);
    return 0;

}

//function to intialise page_table_entries all to -1
void init_page_table(){
    int i;
    for(i=0; i < PAGE_TABLE_SIZE; i++){
        page_table[i].valid = -1;
    }
}