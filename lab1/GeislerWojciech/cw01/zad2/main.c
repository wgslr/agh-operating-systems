#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "../zad1/stringlib.h"

const int TIME_CYCLES = 1000;

typedef struct timeval timeval;
typedef struct timespec timespec;

typedef struct timing {
    timespec system;
    timespec user;
    timespec real;
} timestamp;

void print(char** array, size_t size);

void print_help();

void time_create_dynamic(size_t blocks, size_t block_size);

timespec timeval_to_timespec(timeval time);

timestamp get_timestamp();

// Creates array with every block created and filled with random contents
array* create_filled(size_t blocks, size_t block_size, bool use_static);

void print_timing(timestamp start, timestamp end, unsigned long cycles);

/** Helpers **/
void print_timediff(timespec start, timespec end, const char* description, unsigned cycles);

int main(int argc, char* argv[]) {
    srand(time(NULL));

    size_t blocks = 0;
    size_t block_size = 0;
    bool use_static = false;

    printf("Arguments: %d\n", argc);

    for(int i = 1; i < argc; ++i) {
        if(argv[i][0] != '-') {
            fprintf(stderr, "Incorrect arguments format in arg %d\n", i);
            return (1);
        }
        switch(argv[i][1]) {
            case 'h':
                print_help();
                return (0);
            case 'n':
                blocks = strtoul(argv[i + 1], NULL, 10);
                ++i;
                break;
            case 'b':
                block_size = strtoul(argv[i + 1], NULL, 10);
                ++i;
                break;
            case 'm':
                use_static = (bool) strtoul(argv[i + 1], NULL, 10);
                ++i;
                break;
        }
    }

    time_create_dynamic(blocks, block_size);
    time_search(create_filled(blocks, block_size, false), 0);
    time_search(create_filled(blocks, block_size, true), 0);

    time_add(blocks, block_size);

    return 0;
}

// Creates array with every block created and filled with random contents
array* create_filled(size_t blocks, size_t block_size, bool use_static) {
    array* arr = create_array(blocks, block_size, use_static);
    for(int i = 0; i < blocks; ++i) {
        create_block(arr, i);
        fill_random(get_block(arr, i), arr->block_size);
    }
    return arr;
}

void time_create_dynamic(size_t blocks, size_t block_size) {
    timestamp start = get_timestamp();

    unsigned cycles = (unsigned) (10000000.0 / blocks / block_size * 500); // how many times fits in 5 GiB

    for(int cnt = 0; cnt < cycles; ++cnt) {
        array* arr = create_array(blocks, block_size, false);
        for(int i = 0; i < blocks; ++i) {
            create_block(arr, i);
        }
    }

    print_timing(start, get_timestamp(), cycles);
}

void time_search(array* arr, size_t target) {
    timestamp start = get_timestamp();
    size_t result;
    for(int t = 0; t < TIME_CYCLES; ++t) {
        result = find_nearest(arr, target);
    }
    timestamp end = get_timestamp();
    printf("Block %lu has value most similar to block %lu\n", result, target);

    printf("Timings with %s allocation:\n", arr->use_static ? "static" : "dynamic");
    print_timing(start, end, TIME_CYCLES);
}

void time_add(size_t blocks, size_t block_size) {
    printf("Measuring time of adding %lu blocks:\n", blocks);
    array* arr = create_array(blocks, block_size, false);

    timestamp start = get_timestamp();
    for(int i = 0; i < blocks; ++i) {
        create_block(arr, i);
    }
    timestamp end = get_timestamp();

    print_timing(start, end, 1);
    print_timing(start, end, 1000000);
}


/** Timing functions **/

timestamp get_timestamp() {
    timestamp ts;
    struct rusage ru;
    struct timespec real;

    clock_gettime(CLOCK_REALTIME, &real);
    ts.real = real;

    getrusage(RUSAGE_SELF, &ru);
    ts.system = timeval_to_timespec(ru.ru_stime);
    ts.user = timeval_to_timespec(ru.ru_utime);
    return ts;
}


void print_timing(timestamp start, timestamp end, unsigned long cycles) {
    if(cycles > 1)
        printf("Times averaged over %d cycles:\n", cycles);
    print_timediff(start.user, end.user, "User time", cycles);
    print_timediff(start.system, end.system, "System time", cycles);
    print_timediff(start.real, end.real, "Real time", cycles);
    printf("\n");
}


timespec timeval_to_timespec(timeval time) {
    timespec ts;
    ts.tv_sec = time.tv_sec;
    ts.tv_nsec = time.tv_usec * 1000;
    return ts;
}


/** Helpers **/
void print_timediff(timespec start, timespec end, const char* description, unsigned cycles) {
    long double diff_ns = (end.tv_sec - start.tv_sec) * 1000000000.0 / cycles + (end.tv_nsec - start.tv_nsec) / cycles;
    printf("%11s: %8.5Lf s = %10.5Lf ms = %12.5Lf us = %13.5Lf ns\n", description,
           diff_ns / 1000000000, diff_ns / 1000000, diff_ns / 1000, diff_ns);
}

void print_arr(const array* arr, size_t size) {
    if(!arr->use_static) {
        for(int i = 0; i < size; ++i) {
            if(arr->content[i] != NULL) {
                printf("%d: %p: %s\n", i, arr->content[i], arr->content[i]);
            } else {
                printf("\n");
            }
        }
    }
}

void print_help() {
    printf("Options:\n"
                   "-n blocks count\n"
                   "-b block size\n"
                   "-m mode (0 - dynamic; 1 - static)\n");
}
