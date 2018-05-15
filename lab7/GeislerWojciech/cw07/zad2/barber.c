// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 1

#include <assert.h>
#include "common.h"

sem_t* sems[SEMS];
int shm_fd;
state *shm;

pid_t pop_client(void);

pid_t peek_queue(void);

barber_state barber_sleep(void) {
    shm->b_state = SLEEPING;
    LOG("Going to sleep");

    // state stabilised
    semsignal(sems, BARBER_STATE_LOCK);

    // do sleep
    semwait(sems, CUSTOMER_AVAIL);
    LOG("Waking up");

    semwait(sems, CURRENT_SEATED);

    // assumptions
    assert(shm->seated_client > 0);

    semwait(sems, BARBER_STATE_LOCK);
    return CUTTING;
}

barber_state barber_cut(void) {
    assert(shm->seated_client > 0);

    shm->b_state = CUTTING;
    LOG("Cutting hair of %d", shm->seated_client);
    semsignal(sems, BARBER_STATE_LOCK);

    LOG("Finished cutting hair of %d", shm->seated_client);

    // cause client to go away
    semsignal(sems, FINISHED);

    semwait(sems, LEFT);

    // client should empty the chair
    assert(shm->seated_client <= 0);

    // lock on queue shouldn't be necessary
    // since nonempty queue won't become empty
    // without barber action

    semwait(sems, BARBER_STATE_LOCK);

    if(peek_queue() < 0) {
        return SLEEPING;
    } else {
        return INVITING;
    }
}

barber_state barber_invite(void) {
    assert(shm->seated_client <= 0);

    pid_t next_client;
    sem_t* client_sem;

    shm->b_state=INVITING;
    semsignal(sems, BARBER_STATE_LOCK);

    semwait(sems, QUEUE_LOCK);
    next_client = pop_client();
    semsignal(sems, QUEUE_LOCK);

    LOG("Inviting client %d", next_client)

    client_sem = get_client_sem(next_client);
    semsignal(&client_sem, CLIENT_INVITED);
    // wait for client to sit
    semwait(sems, CURRENT_SEATED);
    assert(shm->seated_client == next_client);


    semwait(sems, BARBER_STATE_LOCK);
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
    OK(munmap(shm, sizeof(state)), "Unmounting shm failed");
    OK(shm_unlink(SHARED_NAME), "Removing shm failed");

    char *semname;
    for(int i = 0; i< SEMS; ++i){
        semname = get_sem_name(i);

        sem_close(sems[i]);
        sem_unlink(semname);
    }
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
    }

    atexit(&cleanup);
    cleanup();

    int chairs = atoi(argv[1]);
    char *semname;

    for(int i = 0; i < SEMS; ++i) {
        semname = get_sem_name(i);
        sems[i] = sem_open(semname, O_RDWR | O_CREAT,  0600u, i == QUEUE_LOCK);
        free(semname);
        if(sems[i] == SEM_FAILED) {
            fprintf(stderr, "Failed opening %dth semafore: %s\n", i, strerror(errno));
            exit(1);
        }
    }

    OK(shm_fd = shm_open(SHARED_NAME, O_RDWR | O_CREAT, 0600u), "Failed creating shared memory");
    OK(ftruncate(shm_fd, sizeof(state)), "Truncating shm failed");

    shm = mmap(NULL, sizeof(state), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm == (void *) -1) {
        fprintf(stderr, "Failed attaching shared memory: %s\n", strerror(errno));
        exit(1);
    }

    shm->queue_length = 0;
    shm->chairs = chairs;
    shm->seated_client = -1;
    for(int i = 0; i < MAX_QUEUE; ++i) {
        shm->queue[i] = -1;
    }

    dispatch();

    return 0;
}