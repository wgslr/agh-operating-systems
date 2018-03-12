#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "../zad1/stringlib.h"

void print(char **array, size_t size);

void print_help() ;

int main(int argc, char *argv[]) {
    srand(time(NULL));

    size_t blocks = 0;
    size_t block_size = 0;
    bool use_static = false;

    printf("Arguments: %d\n", argc);

    for(int i = 1; i < argc; ++i) {
        if(argv[i][0] != '-') {
            fprintf(stderr, "Incorrect arguments format in arg %d\n", i);
            return(1);
        }
        switch(argv[i][1]) {
            case 'h':
                print_help();
                return(0);
            case 'n':
                blocks = strtoul(argv[i + 1], NULL, 10);
                ++i;
                break;
            case 'b':
                block_size = strtoul(argv[i + 1], NULL, 10);
                ++i;
                break;
        }
    }

    fprintf(stderr, "Creating array of %u blocks sized %u\n", blocks, block_size);


    char** arr = get_static();

    printf("%u %u %u\n", &arr, &arr[0], &arr[1]);

    fill_random(&arr[0], 5);
    fill_random(&arr[1], 5);
    fill_random(&arr[3], 7);

    print_arr(&arr, 10);

    return 0;
}



/** Helpers **/

void print_arr(char** array, size_t size) {
    for(int i = 0; i < size; ++i) {
        if(array[i] != NULL) {
            printf("%d: %p: %s\n", i, array[i], array[i]);
        } else {
            printf("\n");
        }
    }
}

void print_help() {
    printf("Options:\n"
                   "-n blocks count\n"
                   "-b block size\n"
                   "-m mode (0 - dynamic; 1 - static)\n");
}


