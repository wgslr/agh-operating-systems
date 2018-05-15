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
    shm->is_sleeping = false;
    shm->current_client = -1;
    for(int i = 0; i < MAX_QUEUE; ++i) {
        shm->queue[i] = -1;
    }

    barber_loop();

    return 0;
}