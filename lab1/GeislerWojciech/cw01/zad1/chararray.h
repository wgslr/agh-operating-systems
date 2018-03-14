//
// Created by wojciech on 3/7/18.
//

#ifndef TASK_STRINGLIB_H
#define TASK_STRINGLIB_H

#include <stdbool.h>

#define MAX_BLOCKS 1000000
#define MAX_BLOCKS_SIZE 1000

typedef struct array {
    char** content;
    size_t blocks;
    size_t block_size;
    bool use_static;
} array;

array* create_array(size_t blocks_count, size_t block_size, bool use_static);

void delete_array(array* arr);

char* create_block(const array* arr, size_t pos);

void delete_block(const array* arr, size_t pos);

char* get_block(array* arr, size_t pos);

void fill(char* block, char* value, size_t size);

void fill_str(char* block, char* value);

void fill_random(char* block, size_t size);

size_t find_nearest(const array* arr, size_t target);

#endif
