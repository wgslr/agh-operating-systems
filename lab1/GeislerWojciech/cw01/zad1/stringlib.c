
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "stringlib.h"

const char MIN_CHAR = 'A';
const char MAX_CHAR = 'z';

char st_array[MAX_BLOCKS][MAX_BLOCKS_SIZE];

char** get_static() {
    printf("%u %u %u\n", &st_array, &st_array[0], &st_array[1]);
    for(size_t i = 0; i < MAX_BLOCKS; ++i) {
        st_array[i][0] = '\0';
    }
    return (char**) &st_array;
}


char** create(size_t count) {
    char** array = calloc(count, sizeof(char*));
    return array;
}

char** delete(char** array, size_t size) {
    for(size_t i = 0; i < size; ++i) {
        free(array[i]);
    }
    free(array);
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

void fill_str(char* block, char* value) {
    size_t size = strlen(value);
    memcpy(block, value, size);
}

void fill_random(char* block, size_t size) {
    for(int i = 0; i < size - 1; ++i) {
        block[i] = (char) (rand() % (MAX_CHAR - MIN_CHAR + 1) + MIN_CHAR);
    }
    block[size - 1] = '\0';
}


size_t find_nearest(char** array, size_t size, size_t target) {
    int target_sum = sum_block(array[target]);

    size_t best_pos = 0;
    int best_diff = INT_MAX;

    for(size_t i = 0; i < size; ++i) {
        if(i != target) {
            int sum = sum_block(array[i]);
            if(abs(sum - target_sum) < best_diff) {
                best_diff = abs(sum - target_sum);
                best_pos = i;
            }
        }
    }
    return best_pos;
}

int sum_block(char* block) {
    int sum = 0;
    for(int i = 0; block[i] != '\0'; ++i) {
        sum += block[i];
    }
    return sum;
}
