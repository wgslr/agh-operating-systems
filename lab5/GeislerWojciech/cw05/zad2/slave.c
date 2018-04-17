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
#include <time.h>


char *get_date(void) {
    FILE *pipe = popen("date", "r");

    char *line = NULL;
    size_t length = 0;
    if(getline(&line, &length, pipe) == -1) {
        fprintf(stderr, "Error reading date pipe: %s\n", strerror(errno));
        exit(1);
    }
    pclose(pipe);
    return line;
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr, "Pipe file is required as the first argument and print count as second\n");
        exit(1);
    }

    srand((unsigned int) time(NULL) * getpid());

    FILE *handle = fopen(argv[1], "r+");
    const int repeats = atoi(argv[2]);
    char *date;

    if(handle == NULL) {
        fprintf(stderr, "Error opening pipe: %s\n", strerror(errno));
        exit(1);
    }

    printf("Slave %d is born\n", getpid());
    for(int i = 0; i < repeats; ++i) {
        date = get_date();
        fprintf(handle, "%d: %s", getpid(), date);
        fflush(handle);

        free(date);
        sleep(rand() % 4 + 2);
    }


}

