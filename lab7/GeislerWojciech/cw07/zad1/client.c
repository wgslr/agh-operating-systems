//
// Created by wojciech on 5/8/18.
//

#include "common.h"

int semset_id;
int shm_id;
int repeats;
int clients;

void be_client(void) {
    LOG("Client is born")
    while(repeats--) {

    }
}

void spawn(void){
    pid_t pid = fork();
    if(pid == 0) {
        be_client();
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

    clients = atoi(argv[1]);
    repeats = atoi(argv[2]);

    key_t key = get_ipc_key();
    OK(semset_id = semget(key, SEMS, IPC_CREAT | 0600u), "Creating semaphore set failed");
    OK(shm_id = shmget(key, sizeof(state), IPC_CREAT | 0600u), "Failed creating shared memory");


    while(clients--) {
        spawn();
    }

    return 0;
}