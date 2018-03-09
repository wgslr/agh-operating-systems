//
// Created by wojciech on 3/7/18.
//

#ifndef TASK_STRINGLIB_H
#define TASK_STRINGLIB_H

char **create(size_t count);

void free_array(char **array, size_t size);

void set_value(char** array, size_t pos, char* data);


char* create_block(char** array, size_t pos, size_t size);

void fill(char* block, char* value, size_t size);

void fill_random(char* block, size_t size);

size_t find_nearest(char** array, size_t size, size_t target);

void delete_block(char** array, size_t pos);

#endif //TASK_STRINGLIB_H
