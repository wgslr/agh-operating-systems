#include <stdio.h>
#include "../zad1/stringlib.h"

#define BLOCKS 5

int main() {
    char** arr = create(BLOCKS, 10);
    set_value(arr, 2, "abc");
    for(int i = 0; i < BLOCKS; ++i) {
        if(arr[i] != NULL) {
            printf("%s\n", arr[i]);
        } else {
            fprintf(stderr, "Skipping line %d\n", i);
        }
    }
    return 0;
}