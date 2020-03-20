#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define VIRTUAL_PAGE_NUMBER 0xFF00
#define VIRTUAL_OFFSET_NUMBER 0xFF
// #define VIRTUAL_PAGE_SHIFT 8
// #define VIRTUAL_OFFSET_SHIFT 0

#define PAGE_SIZE 256
#define PAGE_ENTRIES 256
#define PAGE_NUM_BITS 8
#define FRAME_SIZE 256
#define FRAME_ENTRIES 256
#define MEM_SIZE (FRAME_SIZE * FRAME_ENTRIES) // memory size in bytes
// #define TLB_ENTRIES 16 // 16 entries in TBL at a time

/* Global variables. */
int virtual_address;  // Virtual address.
int page_number;      // Page number.
int offset_number;    // Offset.
int physical_address; // Physical address.
int frame_number;     // Frame number.
int value_in_frame_number;
int page_table[PAGE_ENTRIES]; // Page table.
int tlb_table[16][2];         // Translation look-aside buffer.
int tlb_front = -1;           // TLB front index, queue data structure for FIFO
int tlb_back = -1;            // TLB back index, queue data structure for FIFO
char memory[MEM_SIZE];        // Physical memory. Each char is 1 byte.
int mem_index = 0;            // Points to beginning of first empty frame.

/* Statistics variables. */
int fault_counter = 0;   // Count the page faults.
int tlb_counter = 0;     // TLB hit counter.
int address_counter = 0; // Counts addresses read from file.
float fault_rate;        // Fault rate.
float tlb_rate;          // TLB hit rate.

int main()
{
    char input_read[8]; // holds each line in a file
    int vaddress;       // its the integer version of the input_read
    int page_number;
    int offset_number;
    int value_in_physical_address;

    FILE *virtual_address_file_pointer;
    FILE *physical_address_file_pointer;
    char char_value_from_address_file;

    virtual_address_file_pointer = fopen("addresses.txt", "r");
    if (virtual_address_file_pointer == NULL)
    {
        printf("Error no virtual file");
    }

    physical_address_file_pointer = fopen("out.txt", "w");

    while (fgets(input_read, sizeof(input_read), virtual_address_file_pointer))
    {
        vaddress = atoi(input_read);
        // printf("%d\n", vaddress);
        // now, given the vaddress, get the page number and the offset
        // page_number = (vaddress & VIRTUAL_PAGE_NUMBER) >> 8;
        page_number = vaddress >> 8;
        // printf("%d\n", page_number);
        offset_number = vaddress & VIRTUAL_OFFSET_NUMBER;
        // printf("%d\n", offset_number);

        // check if page_number is in the TLB_table
        if (checkTLB(page_number) == -1)
        { // if not in TLB_table
            if (page_table[page_number] == -1)
            { // if not in page_table there's a page fault
                fault_counter++;
                int page_address = page_number * PAGE_SIZE;
                if (mem_index != -1)
                { // free frame exists
                   printf("need to work on this");
                }
            }
        }
        else if (checkTLB(page_number) != -1) { // exists in tlb table
            physical_address = frame_number + offset_number;
            value_in_physical_address = memory[physical_address];
        }
        fprintf(physical_address_file_pointer, "Virtual address: %d", vaddress);
        fprintf(physical_address_file_pointer, " Phyiscal address: %d", physical_address);
        fprintf(physical_address_file_pointer, " Value: %d\n", value_in_physical_address);
    }

    return 0;
}

int checkTLB(int page_number)
{
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 1; j++)
        {
            if (tlb_table[i][j] == page_number)
            {
                tlb_counter++;
                return tlb_table[i][j];
            }
        }
    }
}
