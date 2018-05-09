// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L

#include <assert.h>
#include "common.h"

int semset;
int shm_id;
state *shm;
int repeats;

void push_client(void);

void client_loop(void) {
    LOG("Client is born")
    while(repeats--) {

        LOG("Wait for state rw")
        wait(semset, STATE_RWLOCK);
        LOG("Waited for state rw")
        if(shm->is_sleeping) {
            assert(shm->current_client == -1);
            assert(shm->queue_count == 0);
            shm->current_client = getpid();
            signal(semset, CUSTOMER_AVAIL);
            signal(semset, STATE_RWLOCK);
            signal(semset, CURRENT_SEATED);

            wait(semset, INVITATION);
            if(shm->queue[0] == getpid()) {
                LOG("Seating at barber's chair");
                wait(semset, FINISHED);

                wait(semset, STATE_RWLOCK);
                LOG("Exiting shop with new haircut");
                shm->current_client = -1;
                signal(semset, STATE_RWLOCK);

            } else {
                // TODO RISKY!
                signal(semset, INVITATION); // signal for other customers
                wait(semset, INVITATION);
            }
        } else if(shm->queue_count < shm->chairs) {
            push_client();
            LOG("Taking seat in the queue");
            signal(semset, STATE_RWLOCK);
        } else {
            LOG("Exiting because of full queue");
            signal(semset, STATE_RWLOCK);
        }
    }
}

void push_client(void) {
    shm->queue[shm->queue_count++] = getpid();
}

void spawn(void) {
    pid_t pid = fork();
    if(pid == 0) {
        client_loop();
        exit(0);
    } else {
        return;
    };
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Wrong number of arguments!\n");
        exit(1);
    }

    int clients = atoi(argv[1]);
    repeats = atoi(argv[2]);

    key_t key = get_ipc_key();
    OK(semset = semget(key, SEMS, IPC_CREAT | 0600u), "Creating semaphore set failed");
    OK(shm_id = shmget(key, sizeof(state), IPC_CREAT | 0600u), "Failed creating shared memory");
    shm = shmat(shm_id, NULL, 0);
    if(shm == (void *) -1) {
        fprintf(stderr, "Failed attaching shared memory");
        exit(1);
    }


    while(clients--) {
        spawn();
    }

    return 0;
}