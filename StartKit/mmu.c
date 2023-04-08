#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



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
    int last_used;
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

//lru_struct
typedef struct lru_struct{
    int page_number;
    int frame_number;
    int last_used;
} lru_struct;

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

//function prototypes
void init_page_table();
int search_page_table(int page_number);
int read_backing_store(int page_number, int frame_number, char * argv[]);
void init_free_frame_list(char* argv[]);
int return_and_remove_free_frame();
void add_frame_to_tlb(int page_num, int frame_num);
int search_tlb_table(int page_num);
void init_tlb();
lru_struct* lru_pagenumber_and_frame();



int main(int argc, char *argv[]) {

    //intialise page table entries to -1
    init_page_table();
    
    //intialise free frame linked list
    init_free_frame_list(argv);

    init_tlb();

    // pointer to read input file
    FILE *file_ptr;
    char buffer[100];

    //pointer to output file to write to
    FILE *output_file_ptr;
    //check to see if second command line argument is 256 or 128 and output to different files else, print invalid page size
    if (atoi(argv[1]) == 256)
        output_file_ptr = fopen("output256.csv", "w+");
    else if (atoi(argv[1]) == 128)
        output_file_ptr = fopen("output128.csv", "w+");
    else
        printf("Invalid page size");
    // Open file
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
                if (frame== -1){
                    //capture least recently used page number  using lru_pagenumber_and_frame function
                    lru_struct* lru_result = lru_pagenumber_and_frame();
                    //make the frame the new free frame and remove the page number from the page table
                    frame = lru_result->frame_number;
                    page_table[lru_result->page_number].valid_val = -1;
                    //free the lru_result
                    free(lru_result);
                    //set lru_result to NULL
                    lru_result = NULL;

                }
                //read from backing store
                read_backing_store(page_number, frame, argv);
                page_table[page_number].frame_val = frame;
                page_table[page_number].valid_val = 1; 
                page_table[page_number].last_used = memory_accesss_requests;
             
                

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
// printf("page_faults %d", page_faults);
// printf("tlb_hits %d", tlb_hits);
// printf("memory_access_requests %d", memory_accesss_requests);
fprintf(output_file_ptr, "Page Faults Rate, %.2f%%,\n", ((float)page_faults / memory_accesss_requests) * 100);
fprintf(output_file_ptr, "TLB Hits Rate, %.2f%%,", ((float)tlb_hits / memory_accesss_requests) * 100);



    // Close file
    fclose(file_ptr);
    fclose(output_file_ptr);
    return 0;

    
}

void init_tlb() {
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].page_number = -1;
        tlb[i].frame_number = -1;
     
    }
}


//function to intialise page_table_entries all to -1
void init_page_table(){
    int i;
    for(i=0; i < PAGE_TABLE_SIZE; i++){
        page_table[i].valid_val = -1;
        // printf("%d",page_table[i].frame_val);
    }
}

//intialise a free frame linked list from 0 to 255 with the value of frame number being stored in the frame variable
void init_free_frame_list(char* argv[]){
    int i;
    
    //define the size of the free frame linked list from the command line argument
    //convert the char* to an int
    
   int size_of_ffl = atoi(argv[1])-1;
//    printf("%d",size_of_ffl);
    for(i=size_of_ffl; i >= 0; i--){
        free_frame_node *new_node = malloc(sizeof(free_frame_node));
        // printf(new_node->frame);
        new_node->frame = i;
        new_node->next = head;
        head = new_node;
    }
}

int search_tlb_table(int page_num) {
    int i;
    for (i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].page_number == page_num) {
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
    //check if the linked list is empty
    if (head == NULL) {
        // printf("Linked list is empty");
        return -1;
    }
    free_frame_node *temp = head;
    head = head->next;
    int frame = temp->frame;
    free(temp);
    return frame;
}

//find least recently used frame in the page table
lru_struct* lru_pagenumber_and_frame(){
    int i;
    int min_last_used = page_table[0].last_used;

    lru_struct* lru = (lru_struct*) malloc(sizeof(lru_struct));

    for(i=1; i < PAGE_TABLE_SIZE; i++){
        if((page_table[i].last_used < min_last_used)&& page_table[i].valid_val == 1){
                lru->frame_number = page_table[i].frame_val;
                lru->page_number = i;
                lru->last_used = page_table[i].last_used;

           ;
        }
    }
    return lru; 
}

void add_frame_to_tlb(int page_num, int frame_num){

// // implement a circular buffer to be used for a FIFO 
tlb_replace_index = curr_tlb_size % TLB_SIZE;
//add frame and page number to the tlb 
tlb[tlb_replace_index].frame_number = frame_num;
tlb[tlb_replace_index].page_number = page_num;

curr_tlb_size++;

  
}
