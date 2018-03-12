//
// Created by wojciech on 3/7/18.
//

#ifndef TASK_STRINGLIB_H
#define TASK_STRINGLIB_H

#define MAX_BLOCKS 1000000
#define MAX_BLOCKS_SIZE 1000

char** get_static();

char** create(size_t count);

char** delete(char** array, size_t size);

char* create_block(char** array, size_t pos, size_t size);

void delete_block(char** array, size_t pos);

void fill(char* block, char* value, size_t size);

void fill_str(char* block, char* value);

void fill_random(char* block, size_t size);

int sum_block(char* block);

size_t find_nearest(char** array, size_t size, size_t target);

#endif
