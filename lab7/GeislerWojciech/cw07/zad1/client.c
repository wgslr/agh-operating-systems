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

        LOG("Entering shop")
        wait(semset, STATE_RWLOCK);
        if(shm->is_sleeping) {
            assert(shm->current_client == -1);
            assert(shm->queue_count == 0);

            LOG("Waking up the barber");
            shm->is_sleeping = false;
            signal(semset, CUSTOMER_AVAIL);
            LOG("Sitting at barber's chair");
            shm->current_client = getpid();
            signal(semset, CURRENT_SEATED);

            // end of modifications
            signal(semset, STATE_RWLOCK);

            // waiting for haircut
            wait(semset, FINISHED);

            // rw lock set in barber
            shm->current_client = -1;
            semctl(semset, CURRENT_SEATED, SETVAL, 0);
            LOG("Exiting shop with new haircut");

        } else if(shm->queue_count < shm->chairs) {
            LOG("Taking seat in the queue");
            push_client();
            signal(semset, STATE_RWLOCK);

            LOG("Wating for personal invitation")
            while(true) {
                LOG("1")
                PRINTSEM
                wait(semset, INVITATION);
                LOG("2")
                PRINTSEM
                if(shm->expected_Client != getpid()) {
//                    LOG("Still waiting")
                    signal(semset, INVITATION); // resume waiting - probably duplicates
                    LOG("3")
                    PRINTSEM
                } else {
                    LOG("Finished queueing")
                    break;
                }

                // DELETE
                sleep(1);
            }

            LOG("DEBUG: wait state_rwlock")
            wait(semset, STATE_RWLOCK);
            assert(shm->current_client == -1);

            LOG("Sitting at barber's chair");
            shm->current_client = getpid();
            signal(semset, CURRENT_SEATED);

            // end of modifications
            signal(semset, STATE_RWLOCK);

            // waiting for haircut
            wait(semset, FINISHED);

            // rw lock set in barber
            shm->current_client = -1;
            semctl(semset, CURRENT_SEATED, SETVAL, 0);
            LOG("Exiting shop with new haircut");
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