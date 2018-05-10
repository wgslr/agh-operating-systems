// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 1

#include <assert.h>
#include <sys/wait.h>
#include "common.h"

int sems;
int shm_id;
state* shm;
int repeats;

void push_client(void);

void client_loop(void) {
    while(repeats--) {
        LOG("Entering shop")
        semwait(sems, QUEUE_STATE);
        if(shm->is_sleeping) {
            assert(shm->current_client == -1);
            assert(shm->queue_count == 0);

            LOG("Waking up the barber");
            shm->is_sleeping = false;
            semsignal(sems, CUSTOMER_AVAIL);
            LOG("Sitting at barber's chair");
            shm->current_client = getpid();
            semsignal(sems, CURRENT_SEATED);

            // end of modifications
            semsignal(sems, QUEUE_STATE);

            // waiting for haircut
            semwait(sems, FINISHED);

            // rw lock is set in barber

            shm->current_client = -1;
            semctl(sems, CURRENT_SEATED, SETVAL, 0);
            LOG("Exiting shop with new haircut");

        } else if(shm->queue_count < shm->chairs) {
            LOG("Taking seat in the queue");
            push_client();
            semsignal(sems, QUEUE_STATE);

            while(true) {
                semwait(sems, INVITATION);
                if(shm->expected_Client != getpid()) {
                    semsignal(sems, INVITATION); // resume waiting
                } else {
                    break;
                }
            }

            semwait(sems, QUEUE_STATE);
            assert(shm->current_client == -1);

            LOG("Sitting at barber's chair");
            shm->current_client = getpid();
            semsignal(sems, CURRENT_SEATED);

            // end of modifications
            semsignal(sems, QUEUE_STATE);

            // waiting for haircut
            semwait(sems, FINISHED);

            // rw lock set in barber
            shm->current_client = -1;
            semctl(sems, CURRENT_SEATED, SETVAL, 0);
            LOG("Exiting shop with new haircut");
        } else {
            LOG("Exiting because of full queue");
            semsignal(sems, QUEUE_STATE);
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

int main(int argc, char* argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Wrong number of arguments!\n");
        exit(1);
    }

    int clients = atoi(argv[1]);
    repeats = atoi(argv[2]);

    key_t key = get_ipc_key();
    OK(sems = semget(key, SEMS, IPC_CREAT | 0600u), "Creating semaphore set failed");
    OK(shm_id = shmget(key, sizeof(state), IPC_CREAT | 0600u), "Failed creating shared memory");
    shm = shmat(shm_id, NULL, 0);
    if(shm == (void*) -1) {
        fprintf(stderr, "Failed attaching shared memory");
        exit(1);
    }


    for(int i = 0; i < clients; ++i) {
        spawn();
    }

    while(clients--) {
        wait(NULL);
    }


    return 0;
}