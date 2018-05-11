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

void semsignal(int semset_id, int sem) {
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


key_t get_ipc_key(int proj_id) {
    const char *home = getenv("HOME");
    return ftok(home, proj_id);
}

int get_client_sem(pid_t pid) {
    int fd;
    OK(fd = semget(get_ipc_key(pid), 1, IPC_CREAT | 0600u), "Creating semaphore set failed");
    return fd;
}


const char * barber_state_to_str(barber_state BS) {
    switch(BS) {
        case SLEEPING:
            return "SLEEPING";
        case INVITING:
            return "INVITING";
        case CUTTING:
            return "CUTTING";
        case WAKING:
            return "WAKING";
        default:
            return "unknown";
    }
}

void print_state(state shm) {
    fprintf(stdout, "b_state: %s\nseated_client: %d\nqueue_length: %d\n",
            barber_state_to_str(shm.b_state), shm.seated_client, shm.queue_length);
}
