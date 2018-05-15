// Wojciech Geisler
// 2018-05

#ifndef LAB7_COMMON_H
#define LAB7_COMMON_H

#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 1

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#define SHARED_NAME "/wgslrapp"
#define MAX_QUEUE 50

#define SEMS 7
#define QUEUE_LOCK 0
#define BARBER_STATE_LOCK 1
#define CUSTOMER_AVAIL 2
#define INVITATION 3
#define CURRENT_SEATED 4
#define FINISHED 5
#define LEFT 6

#define CLIENT_INVITED 0

// Check success and exit with log on error
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

typedef enum barber_state {
    SLEEPING = 10,
    INVITING = 20,
    CUTTING = 30,
    WAKING = 40
} barber_state;

typedef enum client_state {
    OUTSIDE,
    SITTING,
    QUEUING
} client_state;

typedef struct state {
    barber_state b_state;
    pid_t seated_client;

    int queue_length;
    int chairs;
    pid_t queue[MAX_QUEUE];
} state;

key_t get_ipc_key(int proj_id);

void semwait(sem_t* sems[], int sem);

void semsignal(sem_t* sems[], int sem);

sem_t* get_client_sem(pid_t pid);

char* get_sem_name(int id);

#endif //LAB7_COMMON_H
