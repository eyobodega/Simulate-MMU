#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//function prototypes
void init_page_table();
int search_page_table(int page_number);
int read_backing_store(int page_number, int frame_number, char * argv[]);
void init_free_frame_list();
int return_and_remove_free_frame();
void add_frame_to_tlb(int page_num, int frame_num);
int search_tlb_table(int page_num);


//define sizes
#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define NUMBER_OF_FRAMES 256
#define TLB_SIZE 16


//define page entry with default value of valid to -1
typedef struct page_entry {
    int frame_val;
    int valid_val;
} page_table_entry;

//intial page table array as an array of structs page_table_entry
page_table_entry page_table[PAGE_TABLE_SIZE];

//create a free frame linked list
typedef struct free_frame {
   int frame;
    struct free_frame *next;
} free_frame_node;

// pointer to the head of the free frame linked list,intially
free_frame_node *head = NULL;

//define a physical memory data structure
char physical_memory[NUMBER_OF_FRAMES * FRAME_SIZE];

//define a array of structs with int page_number and int frame_number called tlb entry
typedef struct tlb_entry{
    int page_number;
    int frame_number;
} tlb_entry;

//array for struct tlb_entry
tlb_entry tlb[TLB_SIZE];

//tlb control variables 
int curr_tlb_size=0;
int tlb_replace_index=0;

int physical_address;

int frame;

int memory_accesss_requests = 0;
int page_faults = 0;
int tlb_hits = 0;


int main(int argc, char *argv[]) {

    //intialise page table entries to -1
    init_page_table();
    
    //intialise free frame linked list
    init_free_frame_list();

    // pointer to read input file
    FILE *file_ptr;
    char buffer[100];

    //pointer to output file to write to
    FILE *output_file_ptr;
    output_file_ptr = fopen("output256.csv", "w+");

    //could change this to accept a file name as a command line argument
    file_ptr = fopen(argv[3], "r");


    // Read file contents into buffer, parse buffer to get page number and offset
    //change logic to read page_number becuase right shift is not a good idea 
    while (fgets(buffer, sizeof(buffer), file_ptr) != NULL){    
        int address = atoi(buffer);
        int page_number = address >> 8;
        int offset = address & 0xFF;
        memory_accesss_requests++;
        

         frame = search_tlb_table(page_number);
        if (frame != -1){
            physical_address = frame * FRAME_SIZE + offset;
        }
      
        else {
            //search page table for page number, if found return frame number, if not found return -1
             frame = search_page_table(page_number);

            //if frame number is -1, page fault
            if(frame == -1){
                frame = return_and_remove_free_frame();
                //read from backing store
                read_backing_store(page_number, frame, argv);
                page_table[page_number].frame_val = frame;
                page_table[page_number].valid_val = 1; 
             
                

            }
            add_frame_to_tlb(page_number, frame);
            physical_address = (frame * FRAME_SIZE) + offset;
            // printf("physical address %d", physical_address);
            
        }
        // printf("Virtual address: %d Physical address: %d val at address %d\n",
        //      address, physical_address,physical_memory[physical_address]);
        fprintf(output_file_ptr,"%d,%d,%d\n", address, physical_address, physical_memory[physical_address]);
            // printf("physical address %d", physical_address);


    }
printf("page_faults %d", page_faults);
printf("tlb_hits %d", tlb_hits);
printf("memory_access_requests %d", memory_accesss_requests);
fprintf(output_file_ptr, "Page Faults Rate, %.2f%%,\n", ((float)page_faults / memory_accesss_requests) * 100);
fprintf(output_file_ptr, "TLB Hits Rate, %.2f%%,", ((float)tlb_hits / memory_accesss_requests) * 100);



    // Close file
    fclose(file_ptr);
    fclose(output_file_ptr);
    return 0;

    
}

//function to intialise page_table_entries all to -1
void init_page_table(){
    int i;
    for(i=0; i < PAGE_TABLE_SIZE; i++){
        page_table[i].valid_val = -1;
    }
}

//intialise a free frame linked list from 0 to 255 with the value of frame number being stored in the frame variable
void init_free_frame_list(){
    int i;
    for(i=255; i >= 0; i--){
        free_frame_node *new_node = malloc(sizeof(free_frame_node));
        new_node->frame = i;
        new_node->next = head;
        head = new_node;
    }
}

int search_tlb_table(int page_num){
    int i;
    for (i=0; i<TLB_SIZE; i++){
        if (tlb[i].page_number == page_num){
            tlb_hits++;
            return tlb[i].frame_number;
         
        }
    }
    return -1;
    
}

//function to search page table for valid page number, return frame number if found and if not found return -1
int search_page_table(int page_number){
    if (page_table[page_number].valid_val == 1){
        return page_table[page_number].frame_val;
    }
    page_faults++;
    return -1;

}


//function to read backing store, we call this function when main recieves frame == -1 and now call this so that we can find the frame from backing store
int read_backing_store(int page_number, int frame_number, char * argv[]){
 // Open the BACKING_STORE.bin file
    FILE *backing_store_ptr = fopen(argv[2], "rb");

    // Check if the file was opened successfully
    if (backing_store_ptr == NULL) {
        printf("Failed to open BACKING_STORE.bin file.\n");
        return 1;
    }

    // Seek to the beginning of the file
    fseek(backing_store_ptr, page_number*PAGE_SIZE, SEEK_SET);

    // Read one page from the file
    int physical_memory_offset = frame_number * FRAME_SIZE;
    fread(physical_memory + physical_memory_offset, 1, PAGE_SIZE, backing_store_ptr);

    // Close the file
    fclose(backing_store_ptr);

    return 0;

}



//function to remove first free frame from free frame linked list and return it to be used
int return_and_remove_free_frame() {
    free_frame_node *temp = head;
    head = head->next;
    int frame = temp->frame;
    free(temp);
    return frame;
}

void add_frame_to_tlb(int page_num, int frame_num){

// implement a circular buffer to be used for a FIFO 
tlb_replace_index = curr_tlb_size % TLB_SIZE;
//add frame and page number to the tlb 
tlb[tlb_replace_index].frame_number = frame_num;
tlb[tlb_replace_index].page_number = page_num;

curr_tlb_size++;

    
}