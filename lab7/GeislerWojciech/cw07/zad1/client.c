//
// Created by wojciech on 5/8/18.
//

#include "common.h"

int semset_id;
int shm_id;
state *shared_state;
int repeats;

void push_client(void);

void be_client(void) {
    LOG("Client is born")
    while(repeats--) {

        //TODO change to baber check
        LOG("Taking a seat in queue");
        push_client();
        struct sembuf sop = {
                .sem_flg = 0,
                .sem_num =  SEAT_TAKEN_SEM,
                .sem_op = 1
        };
        semop(semset_id, &sop, 1);
    }
}

void push_client(void) {
    shared_state->queue[shared_state->seats_taken++] = getpid();
}

void spawn(void) {
    pid_t pid = fork();
    if(pid == 0) {
        be_client();
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
    OK(semset_id = semget(key, SEMS, IPC_CREAT | 0600u), "Creating semaphore set failed");
    OK(shm_id = shmget(key, sizeof(state), IPC_CREAT | 0600u), "Failed creating shared memory");
    shared_state = shmat(shm_id, NULL, 0);
    if(shared_state == (void *) -1) {
        fprintf(stderr, "Failed attaching shared memory");
        exit(1);
    }


    while(clients--) {
        spawn();
    }

    return 0;
}