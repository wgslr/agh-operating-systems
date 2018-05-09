// Wojciech Geisler
// 2018-05

#ifndef LAB7_COMMON_H
#define LAB7_COMMON_H

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <memory.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#define FTOK_PROJ_ID 123
#define MAX_QUEUE 500

#define SEMS 5
#define CUSTOMER_AVAIL 0
#define STATE_RWLOCK 1
#define CURRENT_SEATED 2
#define INVITATION 3
#define FINISHED 4


// Check success and exit with log on error
#define OK(_EXPR, _ERR_MSG) if((_EXPR) < -1) { fprintf(stderr, "%s: %d %s\n", _ERR_MSG, errno, strerror(errno)); exit(1); }

// Print timestamped message with pid
#define LOG(args...) { \
    struct timespec time; \
    clock_gettime(CLOCK_MONOTONIC, &time); \
    char msg[256]; \
    sprintf(msg, args); \
    printf("%d %ld.%06ld: %s\n", getpid(), time.tv_nsec, time.tv_nsec / 1000, msg); \
}


typedef struct state {
    bool is_sleeping;
    int current_client;
    pid_t queue[MAX_QUEUE];
    int queue_count;
    int chairs;
} state;

key_t get_ipc_key(void);

int wait(int semset_id, int sem);
int signal(int semset_id, int sem);

#endif //LAB7_COMMON_H
