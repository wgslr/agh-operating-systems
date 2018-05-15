// Wojciech Geisler
// 2018-05

#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 1

#include <stdio.h>
#include <time.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"

void signal(sem_t* sems[], int sem) {
    OK(sem_post(sems[sem]), "Semaphore post failed");
}


void semwait(sem_t* sems[], int sem) {
    OK(sem_wait(sems[sem]), "Waiting for semaphore failed");
}

// Returned string must be free'd after use
char* get_sem_name(int id) {
    char *semname = calloc(30, sizeof(char));
    sprintf(semname, "%s%d", SHARED_NAME, id);
    return semname;
}


sem_t* get_client_sem(pid_t pid) {
    sem_t *result;
    char *semname = get_sem_name(SEMS + pid);

    result = sem_open(semname, O_RDWR | O_CREAT,  0600u, 0);
    free(semname);

    if(result == SEM_FAILED) {
        fprintf(stderr, "Failed opening semafore for client %d: %s\n", pid, strerror(errno));
        exit(1);
    }
    return result;
}
