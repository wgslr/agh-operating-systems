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

void signal(int semset_id, int sem) {
    struct sembuf buf = {
            .sem_op = 1,
            .sem_num = sem,
            .sem_flg = 0
    };
    OK(semop(semset_id, &buf, 1), "Signalling semaphore failed");
}


void semwait(int semset_id, int sem) {
    struct sembuf buf = {
            .sem_op = -1,
            .sem_num = sem,
            .sem_flg = 0
    };
    OK(semop(semset_id, &buf, 1), "Waiting semaphore failed");
}

void wait0(int semset_id, int sem) {
    struct sembuf buf = {
            .sem_op = 0,
            .sem_num = sem,
            .sem_flg = 0
    };
    OK(semop(semset_id, &buf, 1), "Wait for semaphore to be 0 failed");
}


key_t get_ipc_key(void) {
    const char *home = getenv("HOME");
    return ftok(home, FTOK_PROJ_ID);
}


