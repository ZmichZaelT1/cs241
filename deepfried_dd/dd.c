/**
 * deepfried_dd
 * CS 241 - Spring 2022
 */
#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "format.h"

static size_t full_blocks_in = 0;
static size_t partial_blocks_in = 0;
static size_t full_blocks_out = 0;
static size_t partial_blocks_out = 0;
static size_t total_bytes_copied = 0;
static clock_t start_time;

void sig_handler() {
    print_status_report(full_blocks_in, partial_blocks_in, full_blocks_out, full_blocks_out, total_bytes_copied, (start_time - clock()) / CLOCKS_PER_SEC);
}

int main(int argc, char **argv) {
    int opt;
    char *i, *o;
    size_t block_size = 512;
    int block_num = -1;
    int block_skip_i = 0;
    int block_skip_o = 0;
    while ((opt = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
        switch(opt) {
            case 'i':
                i = optarg;
                break;
            case 'o':
                o = optarg;
                break;
            case 'b':
                sscanf(optarg, "%zu", &block_size);
                break;
            case 'c':
                sscanf(optarg, "%d", &block_num);
                break;
            case 'p':
                sscanf(optarg, "%d", &block_skip_i);
                break;
            case 'k':
                sscanf(optarg, "%d", &block_skip_o);
                break;
            default:
                exit(1);
        }
    }

    FILE *input_f = stdin;
    FILE *output_f = stdout;
    if (i) {
        input_f = fopen(i, "r");
        if (!input_f) {
            print_invalid_input(i);
            exit(1);
        }
    }

    if (o) {
        output_f = fopen(o, "w+");
        if (!output_f) {
            print_invalid_output(o);
            exit(1);
        }
    }
    
    size_t total_bytes_to_copy;
    if (block_num >= 0) {
        total_bytes_to_copy = block_num * block_size;
    } else {
        fseek(input_f, 0L, SEEK_END);
        total_bytes_to_copy = ftell(input_f) - block_skip_i * block_size;
        rewind(input_f);
    }

    if (input_f != stdin) {
        fseek(input_f, block_skip_i * block_size, SEEK_SET);
    }
    if (input_f != stdout) {
        fseek(output_f, block_skip_o * block_size, SEEK_SET);
    }    
    

    char block_buffer[block_size];
    start_time = clock();
    signal(SIGUSR1, sig_handler);
    while (!feof(input_f) && total_bytes_copied < total_bytes_to_copy) {
        size_t bytes_read = fread(block_buffer, 1, block_size, input_f);
        if (bytes_read <= 0) {
            perror("fread");
            exit(1);
        } else {
            if (bytes_read < block_size) {
                partial_blocks_in++;
            } else {
                full_blocks_in++;
            }
        }
    
        size_t bytes_write = fwrite(block_buffer, 1, bytes_read, output_f);
        if (bytes_write <= 0) {
            perror("fwrite");
            exit(1);
        } else {
            if (bytes_write < block_size) {
                partial_blocks_out++;
            } else {
                full_blocks_out++;
            }
        }
        total_bytes_copied += bytes_write;
    }
    clock_t end_time = clock();
    print_status_report(full_blocks_in, partial_blocks_in, full_blocks_out, partial_blocks_out, total_bytes_copied, (start_time - end_time) / CLOCKS_PER_SEC);
    
    return 0;
}