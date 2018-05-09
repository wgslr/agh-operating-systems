// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 1

#include <assert.h>
#include "common.h"

int semset;
int shm_id;
state *shm;

pid_t pop_client(void);


void barber_loop(void) {
    while(true) {
        semwait(semset, STATE_RWLOCK);

        if(shm->current_client == -1 && shm->queue_count == 0) {
            // nothing to do
            LOG("Going to sleep");
            shm->is_sleeping = true;
            signal(semset, STATE_RWLOCK);

            semwait(semset, CUSTOMER_AVAIL);
            LOG("Waking up");
        } else if(shm->current_client != -1) {
            // already sat when barber slept
            signal(semset, STATE_RWLOCK);

            LOG("Cutting hair of %d", shm->current_client);

            // busy work

            semwait(semset, STATE_RWLOCK);
            LOG("Cut hair of %d", shm->current_client);
            signal(semset, FINISHED);

            // wait for customer to leave
            wait0(semset, CURRENT_SEATED);
            signal(semset, STATE_RWLOCK);

        } else if(shm->queue_count != 0) {
            // pick first one from queue
            pid_t client = shm->expected_Client = pop_client();

            LOG("Inviting client %d", client);

            signal(semset, INVITATION);
            signal(semset, STATE_RWLOCK);

            semwait(semset, CURRENT_SEATED);
            assert(shm->current_client == client);

            // counteract decrement in wait as the seat is still taken
            signal(semset, CURRENT_SEATED);

            LOG("Cutting hair of %d", client);

            // busy work

            semwait(semset, STATE_RWLOCK);
            LOG("Cut hair of %d", shm->current_client);
            signal(semset, FINISHED);

            // wait for customer to leave
            wait0(semset, CURRENT_SEATED);
            signal(semset, STATE_RWLOCK);
        }
    }
}

pid_t pop_client(void) {
    pid_t client = shm->queue[0];

    for(int i = 0; i < shm->queue_count - 1; ++i) {
        shm->queue[i] = shm->queue[i + 1];
    }
    shm->queue_count--;
    return client;
}

void cleanup(void) {
    OK(semctl(semget(get_ipc_key(), 0, 0600u), 0, IPC_RMID), "Deleting semaphore set failed");
    OK(shmdt(shm), "Failed deattaching shm");
    OK(shmctl(shm_id, IPC_RMID, NULL), "Failed deleting shm");
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
    }

    atexit(&cleanup);
    cleanup();

    int chairs = atoi(argv[1]);

    key_t key = get_ipc_key();
    OK(semset = semget(key, SEMS, IPC_CREAT | 0600u), "Creating semaphore set failed");

    OK(shm_id = shmget(key, sizeof(state), IPC_CREAT | 0600u), "Failed creating shared memory");
    shm = shmat(shm_id, NULL, 0);
    if(shm == (void *) -1) {
        fprintf(stderr, "Failed attaching shared memory: %s", strerror(errno));
        exit(1);
    }

    shm->queue_count = 0;
    shm->chairs = chairs;
    shm->is_sleeping = false;
    shm->current_client = -1;
    for(int i = 0; i < MAX_QUEUE; ++i) {
        shm->queue[i] = -1;
    }

    semctl(semset, STATE_RWLOCK, SETVAL, 1);

    barber_loop();

    return 0;
}