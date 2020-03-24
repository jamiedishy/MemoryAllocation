#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFFSET_VALUE 0XFF
#define PAGENUM_VALUE 0XFFFF

// char offset[8];
// char page_number[8];
int tlb_table[16][2];
int page_table[256];
char physical_memory[256][256];
int lru_counter[256]; // to keep track of least recently used

int tlb_first = -1;
int tlb_rear = -1;
int mem_index = 0;

int page_fault_rate = 0;
int tlb_hit_rate = 0;
int total_counter = 0;

char buffer[256];

int main()
{
    char input_read[8];  // holds each line in a file
    
    int virtual_address; // its the integer version of the input_read
    int page_number;
    int offset_number;
    int physical_address;
    int value_in_physical_memory;

    FILE *virtual_address_file_pointer;
    FILE *physical_address_file_pointer;
    FILE *backing_store_pointer;

    virtual_address_file_pointer = fopen("addresses.txt", "r");
    if (virtual_address_file_pointer == NULL)
    {
        printf("Error no virtual file\n");
    }

    backing_store_pointer = fopen("BACKING_STORE.bin", "r");
    if (backing_store_pointer == NULL)
    {
        printf("Error no backing storage\n");
    }

    physical_address_file_pointer = fopen("out.txt", "w");

    init();

    while (fgets(input_read, sizeof(input_read), virtual_address_file_pointer) != 0)
    {
        total_counter++;

        virtual_address = atoi(input_read);
        page_number = (virtual_address & PAGENUM_VALUE) >> 8;
        offset_number = virtual_address & OFFSET_VALUE;

        int frame_number = check_tlb(page_number);

        if (frame_number != -1)
        { // page_number and frame_number are in TLB
            tlb_hit_rate++;
            physical_address = ((frame_number & 0X000000FF ) << 8) | (offset_number & 0X000000FF);
            value_in_physical_memory = physical_memory[frame_number][offset_number];
        }

        else
        {                                                                                                     
            int frame_number = check_page_table(page_number); // check if page_number in page table
            // printf("the frame num is %d\n", frame_number);
            if (frame_number != -1)
            { // page number exists in page table
                physical_address = ((frame_number & 0X000000FF ) << 8) | (offset_number & 0X000000FF);
              
                value_in_physical_memory = physical_memory[frame_number][offset_number];

                // update TLB bc the page_number is not in TLB
                // update_tlb(page_number, frame_number); // FIFO
            }
            else
            { // page_number NOT in page table... page fault
                page_fault_rate++;
                int page_address = page_number * 256; 

                fseek(backing_store_pointer, page_address, SEEK_SET);
                fread(buffer, sizeof(char), 256, backing_store_pointer);

                for (int i = 0; i < 256; i++)
                {
                    physical_memory[mem_index][i] = buffer[i];
                }
                frame_number = mem_index; 
                physical_address = ((frame_number & 0X000000FF ) << 8) | (offset_number & 0X000000FF);
                value_in_physical_memory = physical_memory[frame_number][offset_number];
                page_table[page_number] = frame_number;
                //update_tlb(page_number, frame_number);
                mem_index++;
            }
        }

        fprintf(physical_address_file_pointer, "Virtual address: %d", virtual_address);
        fprintf(physical_address_file_pointer, " Phyiscal address: %d", physical_address);
        fprintf(physical_address_file_pointer, " Value: %d\n", value_in_physical_memory);
    }
    printf("Page Fault Rate:        %d\n", page_fault_rate);
    printf("TLB Hit Rate:           %d\n", tlb_hit_rate);

    return 0;
}

void update_tlb(int pageNumber, int frameNumber)
{ // FIFO Queue
    // printf("hi");
    if (tlb_first == -1)
    {
        tlb_first = 0;
        tlb_rear = 0;
        tlb_table[tlb_rear][0] = pageNumber;
        tlb_table[tlb_rear][1] = frameNumber;
    }
    else
    {
        tlb_rear++;
        if (tlb_rear > 15) { // from 0 to 15 ... size 16
            tlb_rear = 0;
        }
         tlb_table[tlb_rear][0] = pageNumber;
         tlb_table[tlb_rear][1] = frameNumber;
    }
   // return;
}

int check_page_table(int pageNumber)
{
    return page_table[pageNumber];
}

int check_tlb(int pageNumber)
{
    for (int i = 0; i < 16; i++)
    {
        if (tlb_table[i][0] == pageNumber)
        {
            return tlb_table[i][1];
        }
    }
    return -1;
}

void init()
{
    for (int i = 0; i < 16; i++)
    {
        tlb_table[i][0] = -1;
        tlb_table[i][1] = -1;
    }
    for (int i = 0; i < 256; i++)
    {
        page_table[i] = -1;
    }
    // for (int i = 0; i < 256; i++)
    // {
    //     physical_memory[i] = 0;
    // }
}
