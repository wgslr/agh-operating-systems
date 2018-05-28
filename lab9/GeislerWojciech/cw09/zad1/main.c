#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

char **buffer;
bool verbose;

typedef enum {
    SMALLER = -1,
    EQUAL = 0,
    BIGGER = 1
} SEARCH_MODE;

typedef struct {
    char input_path[128];
    int producers_cnt;
    int consumers_cnt;
    int buffor_size;
    int thread_ttl;
    SEARCH_MODE search_mode;
    bool verbose;
} config;


config load_config(const char *path) {
    config c;
    int verbose_int;
    FILE *const f = fopen(path, "r");
    if(f == NULL) {
        fprintf(stderr, "Could not open config file\n");
        exit(1);
    }
    fscanf(f, "%s %d %d %d %d %d %d", c.input_path, &c.producers_cnt, &c.consumers_cnt,
           &c.buffor_size, &c.thread_ttl, &c.search_mode, &verbose_int);
    c.verbose = (bool) verbose_int;
    fclose(f);
    return c;
}


int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Please provide path to config file\n");
        exit(1);
    }

    const config c = load_config(argv[1]);
    verbose = c.verbose;

    if(c.verbose) printf("Allocating buffer of size %d\n", c.buffor_size);
    buffer = calloc(c.buffor_size, sizeof(char *));


    return 0;
}
