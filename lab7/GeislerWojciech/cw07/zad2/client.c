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

void push_client(void) {
    shm->queue[shm->queue_length++] = getpid();
}

void spawn(void) {
    pid_t pid = fork();
    if(pid == 0) {
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

    char *semname;

    for(int i = 0; i < SEMS; ++i) {
        semname = get_sem_name(i);
        sems[i] = sem_open(semname, O_RDWR,  0600u, 0);
        if(sems[i] == SEM_FAILED) {
            fprintf(stderr, "Failed opening %dth semafore\n", i);
            exit(1);
        }
        free(semname);
    }

    OK(shm_fd = shm_open(SHARED_NAME, O_RDWR, 0600u), "Opening shared memory failed");
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