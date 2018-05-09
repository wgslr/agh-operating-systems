// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 1

#include <assert.h>
#include <sys/wait.h>
#include "common.h"

sem_t* sems[SEMS];
int shm_fd;
state *shm;
int repeats;

void push_client(void);

void client_loop(void) {
    while(repeats--) {
        LOG("Entering shop")
        semwait(sems, STATE_RWLOCK);
        if(shm->is_sleeping) {
            assert(shm->current_client == -1);
            assert(shm->queue_count == 0);

            LOG("Waking up the barber");
            shm->is_sleeping = false;
            signal(sems, CUSTOMER_AVAIL);
            LOG("Sitting at barber's chair");
            shm->current_client = getpid();
            signal(sems, CURRENT_SEATED);

            // end of modifications
            signal(sems, STATE_RWLOCK);

            // waiting for haircut
            semwait(sems, FINISHED);

            // rw lock is set in barber

            shm->current_client = -1;

            // reduce to 0
            while(sem_trywait(sems[CURRENT_SEATED]) != -1) {}

            LOG("Exiting shop with new haircut");

        } else if(shm->queue_count < shm->chairs) {
            LOG("Taking seat in the queue");
            push_client();
            signal(sems, STATE_RWLOCK);

            while(true) {
                semwait(sems, INVITATION);
                if(shm->expected_Client != getpid()) {
                    signal(sems, INVITATION); // resume waiting
                } else {
                    break;
                }
            }

            semwait(sems, STATE_RWLOCK);
            assert(shm->current_client == -1);

            LOG("Sitting at barber's chair");
            shm->current_client = getpid();
            signal(sems, CURRENT_SEATED);

            // end of modifications
            signal(sems, STATE_RWLOCK);

            // waiting for haircut
            semwait(sems, FINISHED);

            // rw lock set in barber
            shm->current_client = -1;
            while(sem_trywait(sems[CURRENT_SEATED]) != -1) {}
            LOG("Exiting shop with new haircut");
        } else {
            LOG("Exiting because of full queue");
            signal(sems, STATE_RWLOCK);
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

    char semname[30];

    for(int i = 0; i < SEMS; ++i) {
        sprintf(semname, "/%s%d", SHARED_NAME, i);
        sems[i] = sem_open(semname, O_RDWR,  0600u, i == STATE_RWLOCK);
        if(sems[i] == SEM_FAILED) {
            fprintf(stderr, "Failed opening %dth semafore\n", i);
            exit(1);
        }
    }

    OK(shm_fd = shm_open(SHARED_NAME, O_RDWR, 0600u), "Failed creating shared memory");
    OK(ftruncate(shm_fd, sizeof(state)), "Truncating shm failed");

    shm = mmap(NULL, sizeof(state), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm == (void *) -1) {
        fprintf(stderr, "Failed attaching shared memory: %s\n", strerror(errno));
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