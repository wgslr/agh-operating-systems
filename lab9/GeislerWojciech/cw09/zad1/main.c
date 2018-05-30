#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define OK(_EXPR, _ERR_MSG) if((_EXPR) < 0) { fprintf(stderr, "%s: %d %s\n", _ERR_MSG, errno, strerror(errno)); exit(1); }

// Print timestamped message with pid
#define LOG(_VERBOSE, _PRODUCER, args...) { \
    if(!_VERBOSE || verbose) { \
        struct timespec time; \
        clock_gettime(CLOCK_MONOTONIC, &time); \
        char msg[1024]; \
        sprintf(msg, args); \
        printf("%ld.%06ld %04d %04lu(%c): %s\n", time.tv_sec, time.tv_nsec / 1000, getpid(), pthread_self()%10000, _PRODUCER ? 'P' : 'C', msg); \
        fflush(stdout); \
    }\
}

typedef struct {
    char **buffer;
    size_t next_read;
    size_t next_write; // 1 past last written
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
    int threshold;
    SEARCH_MODE search_mode;
    bool verbose;
} config;

bool matching(const char* line, const config *c);

bool verbose;

buffer buff;

pthread_mutex_t **slot_locks;
pthread_mutex_t *queue_lock;

pthread_mutex_t *filled_slots_mutex;
pthread_cond_t *filled_slots_cond;
int filled_slots;

void mutexunlock(pthread_mutex_t *mutex) {
    OK(pthread_mutex_unlock(mutex), "Unlocking mutex failed");
}


void mutexlock(pthread_mutex_t *mutex) {
    OK(pthread_mutex_lock(mutex), "Locking mutex failed");
}

void *produce(const config *c) {
    LOG(1, 1, "Opening file %s", c->input_path);

    FILE *fd = fopen(c->input_path, "r");

    if(fd == NULL) {
        fprintf(stderr, "Error opening input file for reading: %s\n", strerror(errno));
    }

    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += c->thread_ttl;

    char *line = NULL;
    size_t len = 0;

    while(c->thread_ttl == 0 || time(NULL) <= timeout.tv_sec) {
        if(getline(&line, &len, fd) == -1) {
            LOG(1, 1, "End of file");
            break;
        }

        LOG(1,1,"Waiting for a free place in buffer");

        mutexlock(filled_slots_mutex);
        while(filled_slots == (int)c->buffor_size) {
            if(c->thread_ttl == 0) {
                pthread_cond_wait(filled_slots_cond, filled_slots_mutex);
            } else {
                if(pthread_cond_timedwait(filled_slots_cond, filled_slots_mutex, &timeout) != 0) {
                    LOG(1,1,"Timeout");
                    mutexunlock(filled_slots_mutex);
                    free(line);
                    fclose(fd);
                    pthread_exit(NULL);
                }
            }
        }

        size_t pos = buff.next_write;

        do {
            mutexlock(slot_locks[pos]);

            if(buff.buffer[pos] != NULL) {
                mutexunlock(slot_locks[pos]);
                pos = (pos + 1) % c->buffor_size;
            } else {
                buff.next_write = (pos + 1) % c->buffor_size;

                LOG(1,1, "Writing line at buffer[%zu]", pos);

                buff.buffer[pos] = calloc(strlen(line) + 1 + 0, sizeof(char));
                strcpy(buff.buffer[pos], line);

                LOG(1,1, "Unlocking buffer[%zu]", pos);
                mutexunlock(slot_locks[pos]);

                ++filled_slots;
                pthread_cond_broadcast(filled_slots_cond);
                mutexunlock(filled_slots_mutex);
                break;
            }
        } while(true);
    }
    fclose(fd);
    free(line);
    return NULL;
}


void *consume(const config *c) {
    LOG(1,0, "Consumer is born");

    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += c->thread_ttl;

    while(c->thread_ttl == 0 || time(NULL) <= timeout.tv_sec) {
        LOG(1,0,"Waiting for new portion of information to consume");
        mutexlock(filled_slots_mutex);
        while(filled_slots == 0) {
            if(c->thread_ttl == 0) {
                pthread_cond_wait(filled_slots_cond, filled_slots_mutex);
            } else {
                if(pthread_cond_timedwait(filled_slots_cond, filled_slots_mutex, &timeout) != 0) {
                    LOG(1,0,"Timeout");
                    mutexunlock(filled_slots_mutex);
                    pthread_exit(NULL);
                }
            }
        }
        size_t pos = buff.next_read;

        do {
            mutexlock(slot_locks[pos]);

            if(buff.buffer[pos] == NULL) {
                mutexunlock(slot_locks[pos]);
                pos = (pos + 1) % c->buffor_size;
            } else {
                buff.next_read = (pos + 1) % c->buffor_size;

                if(matching(buff.buffer[pos], c)) {
                    LOG(0, 0, "buffer[%4zu]: %.*s", pos, (int)strlen(buff.buffer[pos]) - 1, buff.buffer[pos]);
                } else {
                    LOG(1, 0, "Skipping buffer[%zu] (condition not met)", pos);
                }

                LOG(1, 0, "Emptying buffer[%zu]", pos);
                free(buff.buffer[pos]);
                buff.buffer[pos] = NULL;

                LOG(1,0, "Unlocking buffer[%zu]", pos);
                mutexunlock(slot_locks[pos]);
                --filled_slots;
                pthread_cond_broadcast(filled_slots_cond);
                mutexunlock(filled_slots_mutex);
                break;
            }
        } while(true);
    }

    return NULL;
}

bool matching(const char* line, const config *c) {
    const int len = (const int) strlen(line);
    switch(c->search_mode) {
        case BIGGER:
            return len > c->threshold;
        case EQUAL:
            return len == c->threshold;
        case SMALLER:
            return len < c->threshold;
        default:
            fprintf(stderr, "Unexpected enum value\n");
            exit(1);
    }
}

config load_config(const char *path) {
    config c;
    int verbose_int;
    FILE *const f = fopen(path, "r");
    if(f == NULL) {
        fprintf(stderr, "Could not open config file\n");
        exit(1);
    }
    if(fscanf(f, "%s %d %d %zu %d %d %d %d", c.input_path, &c.producers_cnt, &c.consumers_cnt,
           &c.buffor_size, &c.thread_ttl, &c.threshold, &c.search_mode, &verbose_int) < 7) {
        fprintf(stderr, "Too few values in config file\n");
        exit(1);
    }

    c.verbose = (bool) verbose_int;
    fclose(f);
    return c;
}

void spawn(config *c) {
    pthread_attr_t *attr = calloc(1, sizeof(pthread_attr_t));

    pthread_t p_tids[c->producers_cnt];
    pthread_t c_tids[c->consumers_cnt];
    pthread_attr_init(attr);

    for(int i = 0; i < c->producers_cnt; ++i) {
        pthread_create(p_tids + i, attr, (void *(*)(void *)) &produce, (void *) c);
    }
    for(int i = 0; i < c->consumers_cnt; ++i) {
        pthread_create(c_tids + i, attr, (void *(*)(void *)) &consume, (void *) c);
    }

    for(int i = 0; i < c->producers_cnt; ++i) {
        pthread_join(p_tids[i], NULL);
    }
    for(int i = 0; i < c->consumers_cnt; ++i) {
        pthread_join(c_tids[i], NULL);
    }

    pthread_attr_destroy(attr);
    free(attr);
}


int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Please provide path to config file\n");
        exit(1);
    }
    config c = load_config(argv[1]);
    verbose = c.verbose;

    buff.buffer = calloc(c.buffor_size, sizeof(char *));
    buff.next_write = 0;
    buff.next_read = 0;

    filled_slots = 0;

    slot_locks = calloc(c.buffor_size, sizeof(pthread_mutex_t *));
    queue_lock = calloc(1, sizeof(pthread_mutex_t));
    filled_slots_mutex = calloc(1, sizeof(pthread_mutex_t));
    filled_slots_cond = calloc(1, sizeof(pthread_cond_t));

    for(size_t i = 0; i < c.buffor_size; ++i) {
        slot_locks[i] = calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(slot_locks[i], NULL);
    }
    pthread_mutex_init(queue_lock, NULL);
    pthread_mutex_init(filled_slots_mutex, NULL);
    pthread_cond_init(filled_slots_cond, NULL);

    spawn(&c);

    for(size_t i = 0; i < c.buffor_size; ++i) {
        pthread_mutex_destroy(slot_locks[i]);
        free(slot_locks[i]);
    }
    free(slot_locks);

    pthread_mutex_destroy(queue_lock);
    pthread_mutex_destroy(filled_slots_mutex);
    pthread_cond_destroy(filled_slots_cond);;
    free(queue_lock);
    free(filled_slots_mutex);
    free(filled_slots_cond);

    return 0;
}
