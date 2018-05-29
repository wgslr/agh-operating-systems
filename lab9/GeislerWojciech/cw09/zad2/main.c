#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <semaphore.h>
#include <bits/time.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define OK(_EXPR, _ERR_MSG) if((_EXPR) < -1) { fprintf(stderr, "%s: %d %s\n", _ERR_MSG, errno, strerror(errno)); exit(1); }

// Print timestamped message with pid
#define LOG(_VERBOSE, _PRODUCER, args...) { \
    if(_VERBOSE || verbose) { \
        struct timespec time; \
        clock_gettime(CLOCK_MONOTONIC, &time); \
        char msg[256]; \
        sprintf(msg, args); \
        printf("%ld.%06ld %lu(%c): %s\n", time.tv_sec, time.tv_nsec / 1000, pthread_self()%10000, _PRODUCER ? 'P' : 'C', msg); \
        fflush(stdout); \
    }\
}

typedef struct {
    char **buffer;
    size_t last_write;
    size_t last_read;
} buffer;

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


bool verbose;

buffer buff;

sem_t **slot_locks;
sem_t *queue_lock;
sem_t *free_slots;

void *produce(const config *c) {
    LOG(1,1, "Read file %s", c->input_path);

    FILE *fd = fopen(c->input_path, "r");

    if(fd == NULL) {
        fprintf(stderr, "Error opening input file for reading: %s\n", strerror(errno));
    }

    const time_t start_time = time(NULL);
    char *line = NULL;
    size_t len = 0;

//    while(c->thread_ttl == 0 || (time(NULL) - start_time) < c->thread_ttl) {
    while(true) {
        // TODO end on ctrl+c
        if(getline(&line, &len, fd) == -1) {
            LOG(1, 1, "End of file");
            break;
        }

//        LOG(1, 1, "Read line of length %zu", strlen(line));
    }
    fclose(fd);
    free(line);
    return NULL;
}


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

    buff.buffer = calloc(c.buffor_size, sizeof(char *));
    buff.last_read = 0;
    buff.last_write = 0;

    slot_locks = calloc(c.buffor_size, sizeof(sem_t*));
    queue_lock = calloc(1, sizeof(sem_t));
    free_slots = calloc(1, sizeof(sem_t));

    for(int i = 0; i < c.buffor_size; ++i) {
        slot_locks[i] = calloc(1, sizeof(sem_t));
        sem_init(slot_locks[i], 0, 1);
    }
//    queue_lock = calloc(1, sizeof(sem_t));
    sem_init(queue_lock, 0, 1);
//    free_slots = calloc(1, sizeof(sem_t));
    sem_init(free_slots, 0, c.buffor_size);

    LOG(1, 0, "Main");

    produce(&c);

    LOG(1, 0, "Freeing");

    for(int i = 0; i < c.buffor_size; ++i) {
        sem_destroy(slot_locks[i]);
        free(slot_locks[i]);
    }
    free(slot_locks);

    sem_destroy(queue_lock);
    sem_destroy(free_slots);
    free(queue_lock);
    free(free_slots);

    return 0;
}
