
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Host.h>
#include "stringlib.h"

//const char MIN_CHAR = 'A';
//const char MAX_CHAR = 'z';
const char MIN_CHAR = 'A';
const char MAX_CHAR = 'C';

char **create(size_t count) {
    char** array = calloc(count, sizeof(char*));
    return array;
}

char* create_block(char** array, size_t pos, size_t size) {
    array[pos] = malloc(size * sizeof(char));
}

void delete_block(char** array, size_t pos) {
    free(array[pos]);
    array[pos] = NULL;
}

void fill(char* block, char* value, size_t size) {
    memcpy(block, value, size);
}

void fill_random(char* block, size_t size) {
    for(int i = 0; i < size; ++i){
        block[i] = (char) (rand() % (MAX_CHAR - MIN_CHAR + 1) + MIN_CHAR);
    }
}

int sum_block(char* block) {
    int sum = 0;
    for(int i = 0; block[i] != '\0'; ++i) {
        sum += block[i];
    }
    return sum;
}

size_t find_nearest(char** array, size_t size, size_t target) {
    int target_sum = sum_block(array[target]);

    size_t best_pos = 0;
    int best_diff = INT_MAX;

    for(int i = 0; i < size; ++i){
        if(i == target)
            continue;
        int sum = sum_block(array[i]);
        printf("Sum of %u: %d\n", i, sum);

        if(abs(sum - target_sum) < best_diff) {
            best_diff = abs(sum - target_sum);
            best_pos = i;
        }
    }
    return best_pos;
}



void free_array(char **array, size_t size) {
    for(int i = 0; i < size; ++i){
        if(array[i] != NULL) {
            free(array[i]);
        }
    }
    free(array);
}

void set_value(char** array, size_t pos, char* data) {
    size_t len = strlen(data);
    array[pos] = malloc(len * sizeof(char));
    memcpy(array[pos], data, len);
}

