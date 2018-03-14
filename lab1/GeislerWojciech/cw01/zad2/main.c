#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "../zad1/stringlib.h"

void print(char** array, size_t size);

void print_help();

void time_create_dynamic(size_t blocks, size_t block_size);

struct timeval timespec_to_timeval(struct timespec time);

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

//    fprintf(stderr, "Creating array of %u blocks sized %u\n", blocks, block_size);
//
//    array* arr = create_array(blocks, use_static);
//    create_block(arr, 0, block_size);
//    fill_random(get_block(arr, 0), block_size);
//    printf("%s\n", get_block(arr, 0));

    time_create_dynamic(blocks, block_size);

    return 0;
}

void time_create_dynamic(size_t blocks, size_t block_size) {
    struct timespec real_start;
    struct timespec real_end;
    struct rusage rusage_start;
    struct rusage rusage_end;
    clock_gettime(CLOCK_REALTIME, &real_start);
    getrusage(RUSAGE_SELF, &rusage_start);

    for(int cnt = 0; cnt < 100; ++cnt) {
        array* arr = create_array(blocks, false);
        for(int i = 0; i < blocks; ++i) {
            create_block(arr, i, block_size);
        }
    }


    clock_gettime(CLOCK_REALTIME, &real_end);
    getrusage(RUSAGE_SELF, &rusage_end);
    print_timing(timespec_to_timeval(real_start), timespec_to_timeval(real_end), "Real time");
    print_timing(rusage_start.ru_utime, rusage_end.ru_utime, "User time");
    print_timing(rusage_start.ru_stime, rusage_end.ru_stime, "System time");
}


/** Helpers **/

void print_timing(struct timeval start, struct timeval end, const char* description) {
//    long double diff_ms = (end->tv_sec - start->tv_sec) * 1000 + (end->tv_nsec - start->tv_nsec) / 1000.0;

//    printf("Start: %llds %lldns, End: %llds %lldns\n", (long long int) start->tv_sec, (long long int) start->tv_nsec,
//           (long long int) end->tv_sec, (long long int) end->tv_nsec);
    long double diff_us = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
//    long double diff_ms = diff_ns / 1000;
    printf("%s: %.3Lf s = %.3Lf ms = %.3Lf us\n", description, diff_us / 1000000, diff_us / 1000, diff_us);
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

struct timeval timespec_to_timeval(struct timespec time) {
    struct timeval tv;
    tv.tv_sec = time.tv_sec;
    tv.tv_usec = time.tv_nsec / 1000;
    return tv;
}
