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
        msgbuf buf = {0};
        ssize_t read = msgrcv(fifo_id, &buf, sizeof(int), 1, IPC_NOWAIT);
        if(read == ENOMSG) {
            LOG("Going to sleep...");

        } else if (read < 1) {
            fprintf(stderr, "Error reading from fifo queue\n");
            exit(1);
        } else {
            LOG("Inviting client %d", buf.client);
        }
    }
}

void cleanup(void) {
    OK(semctl(semget(get_ipc_key(), 0, 0600u), 0, IPC_RMID), "Deleting semaphore set failed");
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
    }

    atexit(&cleanup);

    int chairs = atoi(argv[1]);

    key_t key = get_ipc_key();
    OK(semset_id = semget(key, SEMS, IPC_CREAT | 0600u), "Creating semaphore set failed");
    OK(fifo_id = msgget(key, IPC_CREAT | 0600u), "Creating fifo queue failed");

    OK(semctl(semset_id, EMPTY_SEATS_SEM, SETVAL, chairs), "Setting semaphore value failed");

    be_barber();

    return 0;
}