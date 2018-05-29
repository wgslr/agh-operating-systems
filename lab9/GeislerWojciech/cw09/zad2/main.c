#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <semaphore.h>

#define OK(_EXPR, _ERR_MSG) if((_EXPR) < -1) { fprintf(stderr, "%s: %d %s\n", _ERR_MSG, errno, strerror(errno)); exit(1); }

// Print timestamped message with pid
#define LOG(args...) { \
    struct timespec time; \
    clock_gettime(CLOCK_MONOTONIC, &time); \
    char msg[256]; \
    sprintf(msg, args); \
    printf("%ld.%06ld %d: %s\n", time.tv_nsec, time.tv_nsec / 1000, getpid(), msg); \
    fflush(stdout); \
}

char **buffer;
bool verbose;

sem_t **buf_sems;
sem_t *main_sem;

typedef enum {
    SMALLER = -1,
    EQUAL = 0,
    BIGGER = 1
} SEARCH_MODE;

typedef struct {
    char input_path[128];
    int producers_cnt;
    int consumers_cnt;
    size_t buffor_size;
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
    fscanf(f, "%s %d %d %zu %d %d %d", c.input_path, &c.producers_cnt, &c.consumers_cnt,
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

    buffer = calloc(c.buffor_size, sizeof(char *));
    buf_sems = calloc(c.buffor_size, sizeof(sem_t*));
    main_sem = calloc(1, sizeof(sem_t));

    for(int i = 0; i < c.buffor_size; ++i) {
        buf_sems[i] = calloc(1, sizeof(sem_t));
        sem_init(buf_sems[i], 0, 1);
    }
    main_sem = calloc(1, sizeof(sem_t));
    sem_init(main_sem, 0, 1);



    return 0;
}
