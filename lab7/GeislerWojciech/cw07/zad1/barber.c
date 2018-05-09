// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L

#include "common.h"

int semset;
int shm_id;
state *shm;

pid_t pop_client(void);

void be_barber(void) {
    while(true) {
        wait(semset, STATE_RWLOCK);
        if(shm->seats_taken != 0) {
            pid_t client = pop_client();

            LOG("Inviting client %d", client);
            signal(semset, INVITATION);
            signal(semset, STATE_RWLOCK);

            wait(semset, CURRENT_SEATED);
            LOG("Cutting hair of %d", client);

            // busy work

            LOG("Cut hair of %d", client);
            signal(semset, FINISHED);
        }



        struct sembuf sop = {
                .sem_num = SEAT_TAKEN_SEM,
                .sem_op = -1,
                .sem_flg = 0
        };
        LOG("Waiting for client...");
        int result = semop(semset, &sop, 1);
        if(result == ENOMSG) {
            LOG("Going to sleep");
            sop.sem_num = CURRENT_SEATED;
            semop(semset, &sop, 1);
            LOG("Waking up");
        } else if(result < 0) {
            fprintf(stderr, "Error waiting for semaphor: %d %s\n", result, strerror(result));
            exit(1);
        } else {
            LOG("Inviting client %d", pop_client());
        }
    }
}

pid_t pop_client(void) {
    // TODO lock on queue

    pid_t client = shm->queue[0];

    for(int i = 0; i < shm->seats_taken - 1; ++i) {
        shm->queue[i] = shm->queue[i + 1];
    }
    shm->seats_taken--;
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

    int chairs = atoi(argv[1]);

    key_t key = get_ipc_key();
    OK(semset = semget(key, SEMS, IPC_CREAT | 0600u), "Creating semaphore set failed");

    OK(shm_id = shmget(key, sizeof(state), IPC_CREAT | 0600u), "Failed creating shared memory");
    shm = shmat(shm_id, NULL, 0);
    if(shm == (void *) -1) {
        fprintf(stderr, "Failed attaching shared memory");
        exit(1);
    }

    shm->seats_taken = 0;
    shm->chairs = chairs;
    shm->current_client = -1;
    for(int i = 0; i < MAX_QUEUE; ++i) {
        shm->queue[i] = -1;
    }


    OK(semctl(semset, EMPTY_SEATS_SEM, SETVAL, chairs), "Setting semaphore value failed");

    be_barber();

    return 0;
}