
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "chararray.h"

const char MIN_CHAR = 'A';
const char MAX_CHAR = 'z';

char static_content[MAX_BLOCKS][MAX_BLOCKS_SIZE];

// Calculates sum of character values in a zero-terminated block
int sum_block(char* block) ;

array* create_array(size_t blocks_count, size_t block_size, bool use_static) {
    array* arr = malloc(sizeof(array));
    arr->use_static = use_static;
    arr->blocks = blocks_count;
    arr->block_size = block_size;
    if(!use_static) {
        arr->content = calloc(blocks_count, sizeof(char*));
    } else {
        arr->content = NULL;
    }
    return arr;
}

void delete_array(array* arr) {
    for(size_t i = 0; i < arr->blocks; ++i) {
        delete_block(arr, i);
    }
    if(!arr->use_static) {
        free(arr->content);
        free(arr);
    }
}

char* get_block(array* arr, size_t pos) {
    if(!arr->use_static) {
        return arr->content[pos];
    } else {
        return static_content[pos];
    }
}

// returns pointer to the created block
char* create_block(const array* arr, size_t pos) {
    if(!arr->use_static) {
        arr->content[pos] = calloc(arr->block_size, sizeof(char));
        return arr->content[pos];
    } else {
        // simulate memory zeroed by calloc
        memset(static_content[pos], '\0', arr->block_size);
        return &static_content[pos][0];
    }
}

void delete_block(const array* arr, size_t pos) {
    if(!arr->use_static) {
        free(arr->content[pos]);
        arr->content[pos] = NULL;
    } else {
        static_content[pos][0] = 0;
    }
}

// Fills given block with contents from `value`
// assuming its size to be `size`.
void fill(char* block, char* value, size_t size) {
    memcpy(block, value, size);
}

// Fills given block with contents from `value`
// detecting its size as for a null-terminated string.
void fill_str(char* block, char* value) {
    size_t size = strlen(value);
    memcpy(block, value, size);
}

// Fills given block of `size` characters with
// random characters from a subset of ASCII.
void fill_random(char* block, size_t size) {
    for(size_t i = 0; i < size - 1; ++i) {
        block[i] = (char) (rand() % (MAX_CHAR - MIN_CHAR + 1) + MIN_CHAR);
    }
    block[size - 1] = '\0';
}


size_t find_nearest(const array* arr, size_t target) {
    int target_sum;
    int best_diff = INT_MAX;
    size_t best_pos = 0;

    if(arr->use_static) {
        target_sum = sum_block(static_content[target]);
    } else {
        target_sum = sum_block(arr->content[target]);
    }

    if(!arr->use_static) {
        for(size_t i = 0; i < arr->blocks; ++i) {
            if(i != target) {
                int diff = abs(sum_block(arr->content[i]) - target_sum);

                if(diff < best_diff) {
                    best_diff = diff;
                    best_pos = i;
                }
            }
        }
    } else {
        // Duplicated code to avoid if(use_static) in every loop cycle
        for(size_t i = 0; i < arr->blocks; ++i) {
            if(i != target) {
                int diff = abs(sum_block(static_content[i]) - target_sum);

                if(diff < best_diff) {
                    best_diff = diff;
                    best_pos = i;
                }
            }
        }
    }
    return best_pos;
}

// Calculates sum of character values in a zero-terminated block
int sum_block(char* block) {
    int sum = 0;
    for(int i = 0; block[i] != '\0'; ++i) {
        sum += block[i];
    }
    return sum;
}
