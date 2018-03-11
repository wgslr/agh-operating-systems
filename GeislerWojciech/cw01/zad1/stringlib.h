//
// Created by wojciech on 3/7/18.
//

#ifndef TASK_STRINGLIB_H
#define TASK_STRINGLIB_H

#define MAX_BLOCKS 1024
#define MAX_BLOCKS_SIZE 1024

typedef struct static_array {
    size_t size;
    size_t block_size;
    char array[MAX_BLOCKS][MAX_BLOCKS_SIZE];
} static_array;

static_array static_create();


char **create(size_t count);

void free_array(char **array, size_t size);

void set_value(char** array, size_t pos, char* data);


char* create_block(char** array, size_t pos, size_t size);

void fill(char* block, char* value, size_t size);

void fill_random(char* block, size_t size);

size_t find_nearest_struct(static_array* array, size_t size, size_t target);

size_t find_nearest(char** array, size_t size, size_t target);

void delete_block(char** array, size_t pos);

#endif //TASK_STRINGLIB_H
