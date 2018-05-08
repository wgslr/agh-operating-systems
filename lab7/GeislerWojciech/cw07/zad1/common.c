//
// Created by wojciech on 5/8/18.
//

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <time.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include "common.h"

#define SEMS 3

void logmsg(const char *msg) {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    printf("%ld.%06ld: %s\n", time.tv_nsec, time.tv_nsec / 1000, msg);
}

key_t get_ipc_key(void) {
    const char *home = getenv("HOME");
    return ftok(home, FTOK_PROJ_ID);
}


