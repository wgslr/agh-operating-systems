// Wojciech Geisler

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#define _XOPEN_SOURCE

#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#define PRINT_SPAWN 1
#define PRINT_REQUEST 1
#define PRINT_ALLOWS 1
#define PRINT_SIGRT 1
#define PRINT_CHLD 1

int N, K;
pid_t *children;
bool *allowed;
int request_count = 0;
int alive = 0;


pid_t spawn_child(void);

void spawn_children(int count);

void send_allow(pid_t child);

void allow_all(void);

void request_handler(int signal, siginfo_t *info, void *ucontext);

void sigchld_handler(int signal, siginfo_t *info, void *ucontext);

void rt_handler(int signal, siginfo_t *info, void *ucontext);

void sigint_handler(int signal, siginfo_t *info, void *ucontext) ;

int get_child_id(pid_t pid) {
    int id = 0;
    while(id < N && children[id] != pid) ++id;
    if(id < N){
        return id;
    } else {
        return -1;
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Number of children N and number of requests K must be given");
        return 1;
    }

    sigset_t to_mask;
    sigfillset(&to_mask);

    struct sigaction *sg = calloc(1, sizeof(struct sigaction));
    sg->sa_flags = SA_SIGINFO;
    sg->sa_sigaction = &request_handler;
    sg->sa_mask = to_mask;
    sigaction(SIGUSR1, sg, NULL);

    sg->sa_sigaction = &sigchld_handler;
    sigaction(SIGCHLD, sg, NULL);

    sg->sa_sigaction = &sigint_handler;
    sigaction(SIGINT, sg, NULL);

    sg->sa_sigaction = &rt_handler;
    for(int sig = SIGRTMIN; sig <= SIGRTMAX; ++sig) {
        sigaction(sig, sg, NULL);
    }
    free(sg);

    N = atoi(argv[1]);
    K = atoi(argv[2]);
    children = calloc(N, sizeof(pid_t));
    allowed = calloc(N, sizeof(bool));

    spawn_children(N);

    while(alive > 0) { }

    free(children);
    free(allowed);

    return 0;
}


void spawn_children(int count) {
    for(int i = 0; i < count; ++i) {
        children[i] = spawn_child();
        if(PRINT_SPAWN) printf("%d (%3d): Spawned\n", children[i], i);
    }
}

pid_t spawn_child(void) {
    pid_t pid = fork();
    if(pid == 0) {
        execl("./child", "child", NULL);
        exit(1);
    } else {
        ++alive;
        return pid;
    }
}

void request_handler(int signal, siginfo_t *info, void *ucontext) {
    assert(signal == SIGUSR1);
    (void) ucontext; // unused


    pid_t caller = info->si_pid;
    int childnum = get_child_id(caller);

    if(PRINT_REQUEST) printf("%d (%3d): Requested permission\n", caller, childnum);


    if(!allowed[childnum]) {
        ++request_count;
        allowed[childnum] = true;
    }

    if(request_count == K) {
        allow_all();
    } else if(request_count > K) {
        send_allow(caller);
    }
}

void allow_all(void) {
    for(int i = 0; i < N; ++i) {
        if(allowed[i]) {
            send_allow(children[i]);
        }
    }
}

void send_allow(pid_t child) {
    if(PRINT_ALLOWS) printf("%d (%3d): Sending permission\n", child, get_child_id(child));
    kill(child, SIGALRM);
}

void sigchld_handler(int signal, siginfo_t *info, void *ucontext) {
    assert(signal == SIGCHLD);
    (void) ucontext; // unused

    int left = --alive;
    int status = info->si_status;
    pid_t child = info->si_pid;
    int i = get_child_id(child);

    children[i] = 0;

    assert(status <= 10); // maximum selected time
    if(PRINT_CHLD) printf("%d (%3d): Exited with %d, %d children left\n",
                          info->si_pid, i, status, left);

    // collect zombie
    waitpid(info->si_pid, NULL, 0);
}

void rt_handler(int signal, siginfo_t *info, void *ucontext) {
    assert(signal >= SIGRTMIN && signal <= SIGRTMAX);
    if(PRINT_SIGRT) printf("%d (%3d): Received SIGRT%d\n", info->si_pid, get_child_id(info->si_pid), signal - SIGRTMIN);
}

void sigint_handler(int signal, siginfo_t *info, void *ucontext) {
    for(int i = 0; i < N; ++i){
        if(children[i] != 0){
            kill(children[i], SIGKILL);
            if(PRINT_CHLD) printf("%d (%3d): Killed upon parent exit\n", children[i], i);
        }
    }
    exit(1);
}
