// Wojciech Geisler 2018

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>


int main(int argc, char* argv[]) {
    if(argc < 1) {
        fprintf(stderr, "Pipe file is required as the first argument\n");
        exit(1);
    }

    if(mkfifo(argv[1], 0644) != 0){
        printf("Error creating pipe: %s\n", strerror(errno));
        exit(1);
    }




}

