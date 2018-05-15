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

// about this function's handling of locks I am most unsure
client_state client_come_in(void) {
    semwait(sems, BARBER_STATE_LOCK);
    if(shm->b_state == SLEEPING) {
        LOG("Waking up barber");
        shm->b_state = WAKING; // ugly
        semsignal(sems, CUSTOMER_AVAIL);
        semsignal(sems, BARBER_STATE_LOCK);

        semwait(sems, BARBER_STATE_LOCK);
        return SITTING;
    } else {

        // To prevent deadlock no one must wait for barber having queue
        semwait(sems, QUEUE_LOCK);
        if(shm->queue_length == shm->chairs) {
            LOG("Exiting because of full queue");
            semsignal(sems, BARBER_STATE_LOCK);
            semsignal(sems, QUEUE_LOCK);
            return OUTSIDE;
        } else {
            return QUEUING;
        }
    }
}

client_state client_enqueue(void){
    // queue_state should be locked by previous state
    push_client();
    LOG("Sitting in queue");

    semsignal(sems, QUEUE_LOCK);
    semsignal(sems, BARBER_STATE_LOCK);

    sem_t* client_sem = get_client_sem(getpid());
    semwait(&client_sem, CLIENT_INVITED);
    // invited

    // client semaphore no longer needed
    remove_client_sem(getpid());

    semwait(sems, BARBER_STATE_LOCK);
    return SITTING;
}

client_state client_sit(void) {
    assert(shm->seated_client <= 0);

    LOG("Sitting at chair");
    shm->seated_client = getpid();
    semsignal(sems, BARBER_STATE_LOCK);
    semsignal(sems, CURRENT_SEATED);

    semwait(sems, FINISHED);
    --repeats;
    LOG("Exiting shop with new haircut");

    shm->seated_client = -1;
    semsignal(sems, LEFT);

    return OUTSIDE;
}

void dispatch(void) {
    client_state next_state = OUTSIDE;
    while(true){
        switch(next_state) {
            case OUTSIDE:
                if(repeats) {
                    next_state = client_come_in();
                } else {
                    exit(0);
                }
                break;
            case QUEUING:
                next_state = client_enqueue();
                break;
            case SITTING:
                next_state = client_sit();
                break;
            default:
                assert(false);
        }

    }
}

void push_client(void) {
    shm->queue[shm->queue_length++] = getpid();
}

void spawn(void) {
    pid_t pid = fork();
    if(pid == 0) {
        dispatch();
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