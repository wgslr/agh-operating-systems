#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <dlfcn.h>
#include "../zad1/chararray.h"

#define DL_NAME "libchararray.so"

#define ALLOWED_MEMORY 500000000.0 // 500 MiB

const int TIME_CYCLES = 10000;

typedef struct timeval timeval;
typedef struct timespec timespec;

typedef struct timing {
    timespec system;
    timespec user;
    timespec real;
} timestamp;


// Pointers to functions to allow dynamic loading
typedef struct functions {

    array* (* create_array)(size_t, size_t, bool);

    void (* delete_array)(array*);

    char* (* create_block)(const array*, size_t);

    void (* delete_block)(const array*, size_t);

    char* (*get_block)(array*, size_t);

    void (* fill)(char*, char*, size_t);

    void (* fill_str)(char*, char*);

    void (* fill_random)(char*, size_t);

    size_t (* find_nearest)(const array*, size_t);
} functions;

void print(char** array, size_t size);

void print_help();

void time_create(size_t blocks, size_t block_size, bool use_static, functions funcs);

timespec timeval_to_timespec(timeval time);

timestamp get_timestamp();

// Creates array with every block created and filled with random contents
array* create_filled(size_t blocks, size_t block_size, bool use_static, functions funcs);

void print_timing(timestamp start, timestamp end, unsigned int cycles);

/** Helpers **/
void print_timediff(timespec start, timespec end, const char* description, unsigned cycles);

void time_delete(const array* arr, size_t blocks, functions funcs);

void time_search(array* arr, size_t target, functions funcs);

void time_add(const array* arr, size_t blocks, functions funcs);

void time_add_delete(const array* arr, unsigned cycles, functions funcs);

int main(int argc, char* argv[]) {
    srand(time(NULL));
    functions funcs;
#ifdef DL
    void *handle = dlopen(DL_NAME, RTLD_LAZY);
    if(!handle) {
        fprintf(stderr, "Could not open library %s: %s\n", DL_NAME, dlerror());
        return(1);
    }
    funcs.create_array = dlsym(handle, "create_array");
    funcs.delete_array = dlsym(handle, "delete_array");
    funcs.create_block = dlsym(handle, "create_block");
    funcs.delete_block = dlsym(handle, "delete_block");
    funcs.get_block = dlsym(handle, "get_block");
    funcs.fill = dlsym(handle, "fill");
    funcs.fill_str = dlsym(handle, "fill_str");
    funcs.fill_random = dlsym(handle, "fill_random");
    funcs.find_nearest = dlsym(handle, "find_nearest");
#else
    funcs.create_array = &create_array;
    funcs.delete_array = &delete_array;
    funcs.create_block = &create_block;
    funcs.delete_block = &delete_block;
    funcs.get_block = &get_block;
    funcs.fill = &fill;
    funcs.fill_str = &fill_str;
    funcs.fill_random = &fill_random;
    funcs.find_nearest = &find_nearest;
#endif

    if(argc < 4) {
        print_help();
        return (2);
    }

    size_t blocks = strtoul(argv[1], NULL, 10);
    size_t block_size = strtoul(argv[2], NULL, 10);
    bool use_static = argv[3][0] != '0';


    int arg = 4;
    for(int i = 0; i < 3 && arg < argc; ++i, ++arg) { // 3 commands are expected
        const char* command = argv[arg];
        if(strcmp(command, "create_table") == 0) {
            time_create(blocks, block_size, use_static, funcs);
        } else if(strcmp(command, "search_element") == 0) {
            size_t target = strtoul(argv[++arg], NULL, 10);
            time_search(create_filled(blocks, block_size, use_static, funcs), target, funcs);
        } else if(strcmp(command, "remove") == 0) {
            size_t count = strtoul(argv[++arg], NULL, 10);
            time_delete(create_filled(blocks, block_size, use_static, funcs), count, funcs);
        } else if(strcmp(command, "add") == 0) {
            size_t count = strtoul(argv[++arg], NULL, 10);
            time_add((*funcs.create_array)(blocks, block_size, use_static), count, funcs);
        } else if(strcmp(command, "remove_and_add") == 0) {
            size_t cycles = strtoul(argv[++arg], NULL, 10);
            time_add_delete((*funcs.create_array)(blocks, block_size, use_static), cycles, funcs);
        } else {
            fprintf(stderr, "Unexpected argument %d\n", arg);
            return (2);
        }
        printf("\n");
    }

#ifdef DL
    dlclose(handle);
#endif

    return 0;
}

// Creates array with every block created and filled with random contents
array* create_filled(size_t blocks, size_t block_size, bool use_static, functions funcs) {
    array* arr = (*funcs.create_array)(blocks, block_size, use_static);
    for(size_t i = 0; i < blocks; ++i) {
        (*funcs.create_block)(arr, i);
        (*funcs.fill_random)((*funcs.get_block)(arr, i), arr->block_size);
    }
    return arr;
}

void time_create(size_t blocks, size_t block_size, bool use_static, functions funcs) {
    unsigned cycles = (unsigned) (ALLOWED_MEMORY / blocks / block_size); // how many times fits in 5 GiB
    if(cycles > TIME_CYCLES)
        cycles = TIME_CYCLES;

    printf("Measuring %s creation time for table of %lu blocks sized %lu in %u cycles...\n",
           use_static ? "static" : "dynamic", blocks, block_size, cycles);

    // Store created arrays to free memory afterwards
    array** arrays = calloc(cycles, sizeof(array*));

    timestamp start = get_timestamp();
    for(unsigned c = 0; c < cycles; ++c) {
        arrays[c] = (*funcs.create_array)(blocks, block_size, use_static);
        for(size_t i = 0; i < blocks; ++i) {
            (*funcs.create_block)(arrays[c], i);
        }
    }
    timestamp end = get_timestamp();

    // clear
    for(unsigned c = 0; c < cycles; ++c) {
        (*funcs.delete_array)(arrays[c]);
    }

    print_timing(start, end, cycles);
}

void time_search(array* arr, size_t target, functions funcs) {
    size_t result = 0;

    printf("Measuring search time among %lu blocks sized %lu...\n", arr->blocks, arr->block_size);

    timestamp start = get_timestamp();
    for(int t = 0; t < TIME_CYCLES; ++t) {
        result = (*funcs.find_nearest)(arr, target);
    }
    timestamp end = get_timestamp();

    printf("Result: block %lu has value most similar to block %lu\n\n", result, target);

    printf("Timings with %s allocation - ", arr->use_static ? "static" : "dynamic");
    print_timing(start, end, TIME_CYCLES);
}

void time_delete(const array* arr, size_t blocks, functions funcs) {
    assert(blocks <= arr->blocks);

    printf("Measuring time of deleting %lu %s allocated blocks...\n", blocks,
           arr->use_static ? "statically" : "dynamically");

    timestamp start = get_timestamp();
    for(size_t i = 0; i < blocks; ++i) {
        (*funcs.delete_block)(arr, i);
    }
    timestamp end = get_timestamp();

    print_timing(start, end, blocks);
}

void time_add(const array* arr, size_t blocks, functions funcs) {
    assert(blocks <= arr->blocks);

    printf("Measuring time of creating %lu %s allocated blocks...\n", blocks,
           arr->use_static ? "statically" : "dynamically");

    timestamp start = get_timestamp();
    for(size_t i = 0; i < blocks; ++i) {
        (*funcs.create_block)(arr, i);
    }
    timestamp end = get_timestamp();

    print_timing(start, end, blocks);
}

void time_add_delete(const array* arr, unsigned cycles, functions funcs) {
    printf("Measuring time of adding and removing %s allocated block %u times...\n",
           arr->use_static ? "statically" : "dynamically", cycles);

    timestamp start = get_timestamp();
    for(unsigned i = 0; i < cycles; ++i) {
        (*funcs.create_block)(arr, 0);
        (*funcs.delete_block)(arr, 0);
    }
    timestamp end = get_timestamp();

    print_timing(start, end, cycles);
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


void print_timing(timestamp start, timestamp end, unsigned int cycles) {
    if(cycles > 1) {
        printf("Total timing of %u actions:\n", cycles);
        print_timediff(start.user, end.user, "User time", 1);
        print_timediff(start.system, end.system, "System time", 1);
        print_timediff(start.real, end.real, "Real time", 1);
        printf("Timing of one action averaged over %u retries:\n", cycles);
    }
    print_timediff(start.user, end.user, "User time", cycles);
    print_timediff(start.system, end.system, "System time", cycles);
    print_timediff(start.real, end.real, "Real time", cycles);
}


timespec timeval_to_timespec(timeval time) {
    timespec ts;
    ts.tv_sec = time.tv_sec;
    ts.tv_nsec = time.tv_usec * 1000;
    return ts;
}


void print_timediff(timespec start, timespec end, const char* description, unsigned cycles) {
    long double diff_ns = (end.tv_sec - start.tv_sec) * 1000000000.0 / cycles + (end.tv_nsec - start.tv_nsec) / cycles;
    printf("%11s: %8.5Lf s = %10.5Lf ms = %12.5Lf us = %13.5Lf ns\n", description,
           diff_ns / 1000000000, diff_ns / 1000000, diff_ns / 1000, diff_ns);
}

void print_help() {
    printf("Invocation: main <blocks_count> <block_size> <static_allocation> <command> <command_args...>\n"
                   "Available commands:\n"
                   "- create_table\n"
                   "- search_element <index>\n"
                   "- remove <index>\n"
                   "- add <index>\n"
                   "- remove_and_add <retries>\n"
    );
}
