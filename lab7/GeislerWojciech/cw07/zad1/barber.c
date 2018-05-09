// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L

#include <assert.h>
#include "common.h"

int semset;
int shm_id;
state *shm;

pid_t pop_client(void);

void barber_loop(void) {
    while(true) {
        wait(semset, STATE_RWLOCK);
        if(shm->queue_count != 0) {
            pid_t client = pop_client();

            LOG("Inviting client %d", client);

            // only if was sleeping?
            signal(semset, INVITATION);


            signal(semset, STATE_RWLOCK);

            wait(semset, CURRENT_SEATED);
            LOG("Cutting hair of %d", client);

            // busy work

            LOG("Cut hair of %d", client);
            signal(semset, FINISHED);
        } else {
            LOG("Going to sleep");
            shm->is_sleeping = true;
//            assert(shm->current_client == -1);
            signal(semset, STATE_RWLOCK);

            wait(semset, CUSTOMER_AVAIL);
            LOG("Waking up");
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

//    semctl(semset, STATE_RWLOCK, IPC_SET, 1);
        signal(semset, STATE_RWLOCK);

    barber_loop();

    return 0;
}