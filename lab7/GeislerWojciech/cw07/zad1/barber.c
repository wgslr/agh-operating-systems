// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L

#include "common.h"

int semset_id;
int shm_id;
state *shared_state;

pid_t pop_client(void);

void be_barber(void) {
    while(true) {
        struct sembuf sop = {
                .sem_num = SEAT_TAKEN_SEM,
                .sem_op = -1,
                .sem_flg = 0
        };
        LOG("Waiting for client...");
        int result = semop(semset_id, &sop, 1);
        if(result == ENOMSG) {
            LOG("Going to sleep");
            sop.sem_num = WORK_TO_DO;
            semop(semset_id, &sop, 1);
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
    struct sembuf sop = {.sem_num = STATE_LOCK, .sem_op = 1, .sem_flg = 0};

    // TODO lock on queue

    wait(STATE_RWLOCK);
    signal(STATE_RWLOCK);

    pid_t client = shared_state->queue[0];

    for(int i = 0; i < shared_state->seats_taken - 1; ++i) {
        shared_state->queue[i] = shared_state->queue[i + 1];
    }
    shared_state->seats_taken--;
    return client;
}

void cleanup(void) {
    OK(semctl(semget(get_ipc_key(), 0, 0600u), 0, IPC_RMID), "Deleting semaphore set failed");
    OK(shmdt(shared_state), "Failed deattaching shm");
    OK(shmctl(shm_id, IPC_RMID, NULL), "Failed deleting shm");
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
    }

    atexit(&cleanup);

    int chairs = atoi(argv[1]);

    key_t key = get_ipc_key();
    OK(semset_id = semget(key, SEMS, IPC_CREAT | 0600u), "Creating semaphore set failed");

    OK(shm_id = shmget(key, sizeof(state), IPC_CREAT | 0600u), "Failed creating shared memory");
    shared_state = shmat(shm_id, NULL, 0);
    if(shared_state == (void *) -1) {
        fprintf(stderr, "Failed attaching shared memory");
        exit(1);
    }

    shared_state->seats_taken = 0;
    shared_state->queue_size = chairs;
    shared_state->current_client = -1;
    for(int i = 0; i < MAX_QUEUE; ++i) {
        shared_state->queue[i] = -1;
    }


    OK(semctl(semset_id, EMPTY_SEATS_SEM, SETVAL, chairs), "Setting semaphore value failed");

    be_barber();

    return 0;
}