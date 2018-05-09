// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 1

#include <stdio.h>
#include <time.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"

void signal(sem_t* sems[], int sem) {
    OK(sem_post(sems[sem]), "Semaphore post failed");
}


void semwait(sem_t* sems[], int sem) {
    OK(sem_wait(sems[sem]), "Waiting for semaphore failed");
}

void wait0(sem_t* sems[], int sem) {
    // Naive busy waiting for lack of semop(0)

    int val;
    do {
        sem_getvalue(sems[sem], &val);
    } while(val != 0);
}

