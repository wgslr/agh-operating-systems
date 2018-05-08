//
// Created by wojciech on 5/8/18.
//

#ifndef LAB7_COMMON_H
#define LAB7_COMMON_H

#define _POSIX_C_SOURCE 199309L

#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <memory.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#define FTOK_PROJ_ID 123
#define OK(_EXPR, _ERR_MSG) if((_EXPR) < 0) { fprintf(stderr, "%s: %d %s\n", _ERR_MSG, errno, strerror(errno)); exit(1); }

#define LOG(args...) { \
    struct timespec time; \
    clock_gettime(CLOCK_MONOTONIC, &time); \
    char msg[256]; \
    sprintf(msg, args); \
    printf("%d %ld.%06ld: %s\n", getpid(), time.tv_nsec, time.tv_nsec / 1000, msg); \
}


#define SEMS 3
#define EMPTY_SEATS_SEM 0
#define IS_SLEEPING_SEM 1

typedef struct msgbuf {
    long mtype;
    int client;
} msgbuf;

void logmsg(const char *msg);
key_t get_ipc_key(void);

#endif //LAB7_COMMON_H
