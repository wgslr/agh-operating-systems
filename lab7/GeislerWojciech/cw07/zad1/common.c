// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <time.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"

int signal(int semset_id, int sem) {
    struct sembuf buf = {
            .sem_op = 1,
            .sem_num = sem,
            .sem_flg = 0
    };
    return semop(semset_id, &buf, 1);
}


int wait(int semset_id, int sem) {
    struct sembuf buf = {
            .sem_op = -1,
            .sem_num = sem,
            .sem_flg = 0
    };
    return semop(semset_id, &buf, 1);
}


key_t get_ipc_key(void) {
    const char *home = getenv("HOME");
    return ftok(home, FTOK_PROJ_ID);
}


