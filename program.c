#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define bin_offset 256

void converter(int);
int get_bits(int, unsigned, unsigned);
void get_frame_number(int);
void tlb_add(int, char[]);
int tlb_search(int);

struct item {
    int page_number;
    char frame_number[257];
};

char frame_number[257] = "";
int tlb_size = 15, tlb_index = 0;
struct item *tlb;

int main(int argc, char **argv) {
    // first argument (file name)
    char *file_name = argv[1];
    // set default value for file name
    file_name = file_name != NULL ? file_name : "addresses.txt";
    // open file in read mode
    FILE* file = fopen(file_name, "r");
    // exit if file pointer is null
    if(file == NULL) {
        return 1;
    }
    char line[15];
    // tlb init
    tlb = malloc(sizeof(struct item)*tlb_size);
    // read line by line
    while (fgets(line, sizeof(line), file)) {
        int logical_address = atoi(line);
        converter(logical_address);
    }
    // close file handler
    fclose(file);
    return 0;
}

void converter(int logical_address) {
    // int left = get_bits(logical_address, 31, 16)
    int page_number = get_bits(logical_address, 16, 8);
    int offset = get_bits(logical_address, 8, 0);
    int item_index = tlb_search(page_number);
    if(item_index > -1) {
        strcpy(frame_number, tlb[item_index].frame_number);
    } else {
        get_frame_number(page_number); // global variable frame_number will now have value
        tlb_add(page_number, frame_number);
    }
    printf("logical: %d\npage num: %d\noffset: %d\nframe num: %s\n\n\n", logical_address, page_number, offset, frame_number);
}

void get_frame_number(int num) {
    char buffer[bin_offset] = "";
    int read, i, max_count=0;
    FILE *file = fopen("BACKING_STORE.bin", "rb");
    strcpy(frame_number, "");
    for(int x=0; (read = fread(buffer, 1, sizeof(buffer), file)) > 0; x++) {
        if(x == num) {
            for(i = 0; i < bin_offset; i++) {
                if(!(i >= read || buffer[i] == 0)) {
                    char *temp = (char *) malloc(sizeof(char) * 2);
                    sprintf(temp, "%02hhx", (unsigned char)buffer[i]);
                    strcat(frame_number, temp);
                    free(temp);
                }
            }
            break;
        }
    }
    fclose(file);
}

int get_bits(int address, unsigned to, unsigned from) {
    to -= 1;
    unsigned mask = ((1<<(to-from+1))-1) << from;
    return (address & mask) >> from;
}

void tlb_add(int page_number, char *frame_number) {
    if(tlb_index >= (tlb_size - 1)) {
        for(int i=0; i<tlb_size-1; i++) {
            tlb[i].page_number = tlb[i+1].page_number;
            strcpy(tlb[i].frame_number, tlb[i+1].frame_number);
        }
        tlb[tlb_index].page_number = page_number;
        strcpy(tlb[tlb_index].frame_number, frame_number);
    } else {
        tlb[tlb_index].page_number = page_number;
        strcpy(tlb[tlb_index].frame_number, frame_number);
        tlb_index++;
    }
}

int tlb_search(int page_number) {
    for(int i=0; i<=tlb_index; i++) {
        if(tlb[i].page_number == page_number) {
            return i;
        }
    }
    return -1;
}
