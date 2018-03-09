#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../zad1/stringlib.h"

#define BLOCKS 3
#define LEN 2

void print(char **array);

void experiment();
void experiment2();

int main() {
    srand(time(NULL));

    char** arr = create(BLOCKS);
    for(int i = 0; i < BLOCKS; ++i) {
        create_block(arr, i, LEN);
        fill_random(arr[i], LEN);
    }

    print_arr(arr, BLOCKS);

    printf("%u\n", find_nearest(arr, BLOCKS, 3));


    return 0;
}

void print_arr(char** array, int size) {
    for(int i = 0; i < size; ++i) {
        if(array[i] != NULL) {
            printf("%d: %s\n", i, array[i]);
        }
    }
}

void experiment() {
    char* arr = malloc(BLOCKS * LEN * sizeof(char));
    for(int i = 0; i < BLOCKS * LEN; ++i) {
        *(arr + i) = 'A' + i;
    }

    for(int i = 0; i < BLOCKS * LEN; ++i) {
        printf("&arr[%d] = %c\n", i, &arr[i]);
        printf("arr[%d] = %c\n", i, arr[i]);
    }

    for(int i = 0; i < BLOCKS; ++i) {
        for(int j = 0; j < LEN; ++j) {
            printf("arr[%d][%d] = %c\n", i, j, arr[i][j]);
        }
    }
}

