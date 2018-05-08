// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "common.h"

int semset_id;
int fifo_id;

void be_barber(void) {
    while(true) {


    }
}

void cleanup(void) {
    OK(semctl(semget(key, 0, 0600u), 0, IPC_RMID), "Deleting semaphore set failed");
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
    }

    atexit(&cleanup);

    int chairs = atoi(argv[1]);

    // init semaphores

    key_t key = get_ipc_key();
    OK(semset_id = semget(key, SEMS, IPC_CREAT | 0600u), "Creating semaphore set failed");
    OK(fifo_id = msgget(key, IPC_CREAT | 0600u), "Creating fifo queue failed");

    OK(semctl(semset_id, EMPTY_SEATS_SEM, SETVAL, chairs), "Setting semaphore value failed");


    logmsg("Hello, World!");
    logmsg("Hello, World!");
    logmsg("Hello, World!");
    logmsg("Hello, World!");
    logmsg("Hello, World!");
    return 0;
}