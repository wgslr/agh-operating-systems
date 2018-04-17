// Wojciech Geisler 2018

#define _POSIX_C_SOURCE 200809L
#define _XOPENS_OURCE 500

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
#include <fcntl.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
    if(argc < 1) {
        fprintf(stderr, "Pipe file is required as the first argument\n");
        exit(1);
    }

    if(mkfifo(argv[1], 0644) != 0) {
        fprintf(stderr, "Error creating pipe: %s\n", strerror(errno));
        exit(1);
    }

    FILE *handle = fopen(argv[1], "r");
    char *line = NULL;
    size_t length = 0;

    while(true) {
        if(getline(&line, &length, handle) != -1) {
            printf("%s", line);
        }
    }
}
