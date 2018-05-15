// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 1

#include <assert.h>
#include <sys/wait.h>
#include "common.h"

int sems;
int shm_id;
state *shm;

pid_t pop_client(void);

pid_t peek_queue(void);

barber_state barber_sleep(void) {
    shm->b_state = SLEEPING;
    LOG("Going to sleep");

    // state stabilised
    semsignal(sems, BARBER_STATE);

    // do sleep
    semwait(sems, CUSTOMER_AVAIL);
    LOG("Waking up");

    semwait(sems, CURRENT_SEATED);

    // assumptions
    assert(shm->seated_client > 0);

    semwait(sems, BARBER_STATE);
    return CUTTING;
}

barber_state barber_cut(void) {
    assert(shm->seated_client > 0);

    shm->b_state = CUTTING;
    LOG("Cutting hair of %d", shm->seated_client);
    semsignal(sems, BARBER_STATE);

    LOG("Finished cutting hair of %d", shm->seated_client);

    // cause client to go away
    semsignal(sems, FINISHED);

    semwait(sems, LEFT);

    // client should empty the chair
    assert(shm->seated_client <= 0);

    // lock on queue shouldn't be necessary
    // since nonempty queue won't become empty
    // without barber action

    semwait(sems, BARBER_STATE);

    if(peek_queue() < 0) {
        return SLEEPING;
    } else {
        return INVITING;
    }
}

barber_state barber_invite(void) {
    assert(shm->seated_client <= 0);

    pid_t next_client;
    int client_sem;

    shm->b_state=INVITING;
    semsignal(sems, BARBER_STATE);

    semwait(sems, QUEUE_STATE);
    next_client = pop_client();
    semsignal(sems, QUEUE_STATE);

    LOG("Inviting client %d", next_client)

    client_sem = get_client_sem(next_client);
    semsignal(client_sem, CLIENT_INVITED);
    // wait for client to sit
    semwait(sems, CURRENT_SEATED);
    assert(shm->seated_client == next_client);


    semwait(sems, BARBER_STATE);
    return CUTTING;
}

void dispatch(void) {
    barber_state next_state = SLEEPING;
    while(true){
        switch(next_state) {
            case SLEEPING:
                next_state = barber_sleep();
                break;
            case CUTTING:
                next_state = barber_cut();
                break;
            case INVITING:
                next_state = barber_invite();
                break;
            case WAKING:
            default:
                // shouldn't happen
                assert(false);
        }
    }
}

pid_t peek_queue(void){
    if(shm->queue_length == 0) {
        return -1;
    } else {
        return shm->queue[0];
    }
}

pid_t pop_client(void) {
    if(shm->queue_length == 0) {
        return -1;
    } else {
        pid_t client = shm->queue[0];

        for(int i = 0; i < shm->queue_length - 1; ++i) {
            shm->queue[i] = shm->queue[i + 1];
        }
        shm->queue_length--;
        return client;
    }
}

void cleanup(void) {
    OK(semctl(semget(get_ipc_key(FTOK_PROJ_ID), 0, 0600u), 0, IPC_RMID), "Deleting semaphore set failed");
    OK(shmdt(shm), "Failed deattaching shm");
    OK(shmctl(shm_id, IPC_RMID, NULL), "Failed deleting shm");
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
    }

    atexit(&cleanup);
    cleanup(); // to be sure

    int chairs = atoi(argv[1]);

    key_t key = get_ipc_key(FTOK_PROJ_ID);
    OK(sems = semget(key, SEMS, IPC_CREAT | IPC_EXCL | 0600u), "Creating semaphore set failed");

    OK(shm_id = shmget(key, sizeof(state), IPC_CREAT | 0600u), "Failed creating shared memory");
    shm = shmat(shm_id, NULL, 0);
    if(shm == (void *) -1) {
        fprintf(stderr, "Failed attaching shared memory: %s", strerror(errno));
        exit(1);
    }

    shm->queue_length = 0;
    shm->chairs = chairs;
    shm->seated_client = -1;
    for(int i = 0; i < MAX_QUEUE; ++i) {
        shm->queue[i] = -1;
    }

    semctl(sems, QUEUE_STATE, SETVAL, 1);
    semctl(sems, BARBER_STATE, SETVAL, 0);
    semctl(sems, CUSTOMER_AVAIL, SETVAL, 0);
    semctl(sems, CURRENT_SEATED, SETVAL, 0);

    dispatch();

    return 0;
}