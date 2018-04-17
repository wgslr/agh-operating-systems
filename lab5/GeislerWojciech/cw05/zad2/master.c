// Wojciech Geisler 2018

#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE
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


int main(int argc, char* argv[]) {
    if(argc < 1) {
        fprintf(stderr, "Pipe file is required as the first argument\n");
        exit(1);
    }

    if(mkfifo(argv[1], 0644) != 0){
        printf("Error creating pipe: %s\n", strerror(errno));
        exit(1);
    }

    int fd = open(argv[1], O_RDONLY);
    char buff[2];

    while(true){
        fprintf(stderr, "Reading byte\n");
        int b = read(fd, buff, 1);

        fprintf(stderr, "Read %d bytes\n", b);
        buff[2] = '\0';
        write(STDOUT_FILENO, buff, 1);
            usleep(200000);
    }

//
////    int handle = open(argv[1], O_RDONLY);
//    FILE * handle = fopen(argv[1], "r");
//    char *line = NULL;
//
//    size_t length = 0;
//
//    while(true){
//        fprintf(stderr, "Getline\n");
//        if(getline(&line, &length, handle) != -1) {
//            puts(line);
//        } else {
//            usleep(2000000);
//        }
//    }

}
