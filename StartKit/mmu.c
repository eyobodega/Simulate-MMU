#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//function prototypes
void init_page_table();
int search_page_table(int page_number);

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
        int page_number = address >> 8;
        int offset = address & 0xFF;
        
        //search page table for page number, if found return frame number, if not found return -1
        int frame = search_page_table(page_number);

        //if frame number is -1, page fault
        if(frame == -1){
            printf ("page fault for address %d and page number %d and frame %d", address, page_number, frame);
        


    }
    // Close file
    fclose(file_ptr);
    return 0;

}
}

//function to intialise page_table_entries all to -1
void init_page_table(){
    int i;
    for(i=0; i < PAGE_TABLE_SIZE; i++){
        page_table[i].valid = -1;
    }
}

//function to search page table for valid page number, return frame number if found and if not found return -1
int search_page_table(int page_number){
    int i;
    for(i=0; i < PAGE_TABLE_SIZE; i++){
        if(page_table[i].valid == page_number){
            return page_table[i].frame;
        }
    }
    return -1;
}