#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../zad1/stringlib.h"

#define BLOCKS 4
#define LEN 5

void print(char **array);

void experiment();
void experiment2();

char** static_cr(size_t blocks, size_t size);

int main() {
    srand(time(NULL));
//
//    char** arr = create(BLOCKS);
//    for(int i = 0; i < BLOCKS; ++i) {
//        create_block(arr, i, LEN);
//        fill_random(arr[i], LEN);
//    }
//
//    print_arr(arr, BLOCKS);
//
//    printf("Magic begin\n");
//    struct static_array st = static_create();
//    printf("Magic end\n");
//
//    strcpy(st.array[0], "abc");
//    strcpy(st.array[1], "def");
//    strcpy(st.array[2], "ghi");
//    strcpy(st.array[3], "wxyz");
//
//    printf("%u\n", find_nearest(arr, BLOCKS, 2));
//    printf("%u\n", sizeof(struct static_array));
//    printf("%u\n", &st);
//    printf("%u\n", &st.array);
//    printf("%u\n", find_nearest_struct(&(st), BLOCKS, 2));
////    printf("%u\n", find_nearest(&st.array, BLOCKS, 2));
//
//    char** st2 = static_cr(BLOCKS, 5);
//    find_nearest(st2, MAX_BLOCKS, 2);


    return 0;
}


char** static_cr(size_t blocks, size_t block_size) {
    static char array[MAX_BLOCKS][MAX_BLOCKS_SIZE];
    return (char**) &array;
}

void print_arr(char** array, int size) {
    for(int i = 0; i < size; ++i) {
        if(array[i] != NULL) {
            printf("%d: %s\n", i, array[i]);
        }
    }
}
