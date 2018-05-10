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


void barber_loop(void) {
    while(true) {
        semwait(sems, STATE_RWLOCK);

        if(shm->current_client == -1 && shm->queue_count == 0) {
            // nothing to do
            LOG("Going to sleep");
            shm->is_sleeping = true;
            signal(sems, STATE_RWLOCK);

            semwait(sems, CUSTOMER_AVAIL);
            LOG("Waking up");
        } else if(shm->current_client != -1) {
            // already sat when barber slept
            signal(sems, STATE_RWLOCK);

            LOG("Cutting hair of %d", shm->current_client);

            // busy work

            semwait(sems, STATE_RWLOCK);
            LOG("Cut hair of %d", shm->current_client);
            signal(sems, FINISHED);

            // wait for customer to leave
            wait0(sems, CURRENT_SEATED);
            signal(sems, STATE_RWLOCK);

        } else if(shm->queue_count != 0) {
            // pick first one from queue
            pid_t client = shm->expected_Client = pop_client();

            LOG("Inviting client %d", client);

            signal(sems, INVITATION);
            signal(sems, STATE_RWLOCK);

            semwait(sems, CURRENT_SEATED);
            assert(shm->current_client == client);

            // counteract decrement in wait as the seat is still taken
            signal(sems, CURRENT_SEATED);

            LOG("Cutting hair of %d", client);

            // busy work

            semwait(sems, STATE_RWLOCK);
            LOG("Cut hair of %d", shm->current_client);
            signal(sems, FINISHED);

            // wait for customer to leave
            wait0(sems, CURRENT_SEATED);
            signal(sems, STATE_RWLOCK);
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
    OK(munmap(shm, sizeof(state)), "Unmounting shm failed");
    OK(shm_unlink(SHARED_NAME), "Removing shm failed");

    char semname[30];
    for(int i = 0; i< SEMS; ++i){
        sem_close(sems[i]);

        sprintf(semname, "%s%d", SHARED_NAME, i);
        printf("Unlink %s\n", semname);
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
    char semname[30];

    for(int i = 0; i < SEMS; ++i) {
        sprintf(semname, "%s%d", SHARED_NAME, i);
        sems[i] = sem_open(semname, O_RDWR | O_CREAT,  0600u, i == STATE_RWLOCK);
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

    shm->queue_count = 0;
    shm->chairs = chairs;
    shm->is_sleeping = false;
    shm->current_client = -1;
    for(int i = 0; i < MAX_QUEUE; ++i) {
        shm->queue[i] = -1;
    }

    barber_loop();

    return 0;
}