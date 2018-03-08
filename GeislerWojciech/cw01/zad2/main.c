#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../zad1/stringlib.h"

#define BLOCKS 5
#define LEN 10

void print(char **array);

void experiment();
void experiment2();

int main() {
    experiment3();
//    experiment();
//    printf("experiment2:\n");
//    experiment2();
//
//    char **arr = create(BLOCKS);
//    set_value(arr, 2, "abc");
//
//    print(arr);
//
//    printf("Freeing\n");
//    free_array(arr, BLOCKS);
//
//    char s_arr[BLOCKS][LEN];
//    char *string = "alamakota";
//    printf("Len: %d\n", strlen(string));
//
//    memcpy(s_arr[0], string, strlen(string));

//    printf("Copied\n");
//
//    print(s_arr);
//
//    free_array((char **) s_arr, BLOCKS);
//    print(s_arr);

//    print(arr);

    return 0;
}

void print(char **array) {
    printf("printing %d lines\n", BLOCKS);
    for (int i = 0; i < BLOCKS; ++i) {
        printf("Row %d: \n", i);
        if (array[i] != NULL) {
            printf("%s\n", array[i]);
        } else {
            printf("skipping");
        }
    }
}

void experiment() {
    int s1 = 3;
    char arr2d[3][2];
    char* string = "12345";
    for(int i = 0; i < s1; ++i){
        printf("%d\n", arr2d[i]);
    }
    for(int i = 0; i < 6; ++i) {
        *((char*)arr2d + i) = string[i];
    }
    for(int i = 0; i < s1; ++i){
        printf("%d\n", arr2d[i]);
    }
    printf("%s\n", arr2d);
    printf("%c\n", arr2d[1][0]);

}

void experiment2() {
    int s1 = 3;
    int s2 = 2;
    char* string = "12345";
    char** arr2d = calloc(s1, sizeof(char*));

    for(int i = 0; i < s1; ++i){
        printf("%d\n", arr2d[i]);
    }

    for(int i = 0; i < s1 ; ++i){
        arr2d[i] = calloc(s2, sizeof(char));
    }
    for(int i = 0; i < s1; ++i){
        printf("%d\n", arr2d[i]);
    }


    for(int i = 0; i < 6; ++i) {
//        *((char*)arr2d + i) = string[i];
    }
    arr2d[0] = "12";
    arr2d[1] = "34";
    arr2d[2] = "56";

    for(int i = 0; i < s1; ++i){
        for(int j = 0; j < s2; ++j){
            printf("%d, %d: %c\n", i, j, arr2d[i][j]);
        }
    }


//    char* string = "12345";
//    for(int i = 0; i < 6; ++i) {
//        *((char*)arr2d + i) = string[i];
//    }

//    printf("%s\n", arr2d);
}

void experiment3() {
    const int S1 = 3;
    const int S2 = 2;
    printf("\nExperiment 3:\n");

    printf("Static arr1ay:\n");
    char arr1[3][2] = {"01", "23", "45"};
    printf("&arr1: %u\n", &arr1);
    printf("arr1: %u\n", arr1);
    printf("&arr1[0]: %u\n", &arr1[0]);
    printf("arr1[0]: %u\n", arr1[0]);
    printf("&arr1[0][1]: %u\n", &arr1[0][1]);
    printf("arr1[0][0]: %u\n", arr1[0][1]);


    printf("\n");

    printf("Dynamic array:\n");
    char** arr2 = calloc(S1, sizeof(char*));
    for(int i = 0; i < S1; ++i){
        arr2[i] = calloc(S2, sizeof(char));
    }

    printf("arr2: %u\n", arr2);
    printf("&arr2[0]: %u\n", &arr2[0]);
    printf("arr2[0]: %u\n", arr2[0]);
    printf("&arr2[0][1]: %u\n", &arr2[0][1]);

    printf("\n");
    char** arr1p = arr1;
    printf("arr1p: %u\n", arr1p);
    printf("&arr1p[0]: %u\n", &arr1p[0]);
    printf("arr1p[0]: %u\n", arr1p[0]);
    printf("&arr1p[0][1]: %u\n", &arr1p[0][1]);

}
