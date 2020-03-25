#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFFSET_VALUE 0XFF
#define PAGENUM_VALUE 0XFFFF

// char offset[8];
// char page_number[8];
int tlb_table[16][2];
int page_table[256];
char physical_memory[128][256];
int lru_counter[256]; // to keep track of least recently used

int tlb_first = -1;
int tlb_rear = -1;
int mem_index = 0;
int counter = 1;

int page_faults = 0;
int tlb_hits = 1;
// int total_counter = 0;

char buffer[256];

int main()
{
    char input_read[8]; // holds each line in a file

    int virtual_address; // its the integer version of the input_read
    int page_number;
    int offset_number;
    int physical_address;
    int value_in_physical_memory;
    int frame_number;

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

    physical_address_file_pointer = fopen("output.csv", "w");

    init();

    while (fgets(input_read, sizeof(input_read), virtual_address_file_pointer) != 0)
    {
        // total_counter++;

        virtual_address = atoi(input_read);
        page_number = (virtual_address & PAGENUM_VALUE) >> 8;
        offset_number = virtual_address & OFFSET_VALUE;

        // fprintf(physical_address_file_pointer, "the page number is %d\n", page_number);
        lru_counter[page_number] = counter;
        counter++;

        frame_number = check_tlb(page_number);

        if (frame_number != -1)
        { // page_number and frame_number are in TLB
            tlb_hits++;
            physical_address = ((frame_number & 0X000000FF) << 8) | (offset_number & 0X000000FF);
            value_in_physical_memory = physical_memory[frame_number][offset_number];
        }

        else
        {
            frame_number = check_page_table(page_number); // check if page_number in page table

            if (frame_number != -1)
            { // page number exists in page table
                physical_address = ((frame_number & 0X000000FF) << 8) | (offset_number & 0X000000FF);

                value_in_physical_memory = physical_memory[frame_number][offset_number];

                // update TLB bc the page_number is not in TLB
                update_tlb(page_number, frame_number); // FIFO
            }
            else
            { // frame_number NOT in page table (-1) ... page fault
                page_faults++;

                if (mem_index < 128)
                {
                    int page_address = page_number * 256;

                    fseek(backing_store_pointer, page_address, SEEK_SET);
                    fread(buffer, sizeof(char), 256, backing_store_pointer);

                    for (int i = 0; i < 256; i++)
                    {
                        physical_memory[mem_index][i] = buffer[i];
                    }

                    frame_number = mem_index;
                    physical_address = ((frame_number & 0X000000FF) << 8) | (offset_number & 0X000000FF);
                    value_in_physical_memory = physical_memory[frame_number][offset_number];

                    page_table[page_number] = frame_number;

                    update_tlb(page_number, frame_number);

                    mem_index++;
                }

                else // no more free frames
                {    // if mem_index is greater than physical_memory size and hence no more available frames
                    // what's the least recently used page_number? set page_table[LRU_page_number] = -1 and page_table[page_number] = that frame

                    int temp_page_number = get_LRU();

                    page_table[page_number] = page_table[temp_page_number]; // printf("value in temppagenum fram is %d\n", page_table[temp_page_number]);

                    frame_number = page_table[page_number];
                    // fprintf(physical_address_file_pointer, "were in;)))\n");

                    int page_address = page_number * 256;

                    fseek(backing_store_pointer, page_address, SEEK_SET);
                    fread(buffer, sizeof(char), 256, backing_store_pointer);

                    for (int i = 0; i < 256; i++)
                    {
                        physical_memory[frame_number][i] = buffer[i];
                    }

                    // fprintf(physical_address_file_pointer, "the frame num and offset num are %d %d\n", frame_number, offset_number);
                    physical_address = ((frame_number & 0X000000FF) << 8) | (offset_number & 0X000000FF);
                    // fprintf(physical_address_file_pointer, "physical address and the other half is %d %d\n", physical_address,  ((frame_number & 0X000000FF) << 8) | (offset_number & 0X000000FF));

                    value_in_physical_memory = physical_memory[frame_number][offset_number];

                    page_table[temp_page_number] = -1;

                    lru_counter[temp_page_number] = 1000000;

                    update_tlb(page_number, frame_number);
                }
                // fprintf(physical_address_file_pointer, "the page number and frame numebr and offset number are %d %d %d\n",page_number, frame_number, offset_number);
            }
        }

        fprintf(physical_address_file_pointer, "Logical address: %d, Physical address: %d, Value: %d\n", virtual_address, physical_address, value_in_physical_memory);
        // fprintf(physical_address_file_pointer, " Physical address: %d", physical_address);
        // fprintf(physical_address_file_pointer, " Value: %d\n", value_in_physical_memory);
    }
    printf("Page Faults = %d\n", page_faults);
    printf("TLB Hits = %d\n", tlb_hits);

    return 0;
}

int get_LRU()
{
    int minimum = 90000000;
    int temp_page_num;
    for (int i = 0; i < 256; i++)
    {
        //printf("the value in lrucounter si %d\n", lru_counter[i]);
        if (lru_counter[i] < minimum)
        {
            minimum = lru_counter[i];
            temp_page_num = i;
        }
    }
    // printf( "the temp page num is %d and minimum is %d", temp_page_num, minimum);
    return (temp_page_num);
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
        if (tlb_rear > 15)
        { // from 0 to 15 ... size 16
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
    for (int i = 0; i < 256; i++)
    {
        lru_counter[i] = 1000000;
    }
}
