//
// Created by wojciech on 3/7/18.
//

#ifndef TASK_STRINGLIB_H
#define TASK_STRINGLIB_H

char **create(size_t count);

void free_array(char **array, size_t size);

void set_value(char** array, size_t pos, char* data);


#endif //TASK_STRINGLIB_H
