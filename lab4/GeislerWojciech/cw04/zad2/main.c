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

void print_allowed(void) {
    for(int i = 0; i < N; ++i) {
        printf("%d", allowed[i]);
    }
    printf("\n");
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

    N = atoi(argv[1]);
    K = atoi(argv[2]);
    children = calloc(N, sizeof(pid_t));
    allowed = calloc(N, sizeof(bool));

    print_allowed();

    spawn_children(N);

    while(alive > 0) { }

    return 0;
}


void spawn_children(int count) {
    for(int i = 0; i < count; ++i) {
        children[i] = spawn_child();
        if(PRINT_SPAWN) printf("Spawned %d\n", children[i]);
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

    if(PRINT_REQUEST) printf("Received request from %d\n", caller);

    int childnum = 0;
    while(childnum < N && children[childnum] != caller) ++childnum;

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
    if(PRINT_ALLOWS) printf("Sending allowance to %d\n", child);
    kill(child, SIGALRM);
}

void sigchld_handler(int signal, siginfo_t *info, void *ucontext) {
    assert(signal == SIGCHLD);
    (void) ucontext; // unused

    int left = --alive;
    int status = info->si_status;
    pid_t child = info->si_pid;
    int i = 0;
    while(children[i] != child && i < N) ++i;
    children[i] = 0;

    assert(status <= 10); // maximum selected time
    if(PRINT_CHLD) printf("Child %d exited with %d, %d children left\n", info->si_pid, status, left);

    // collect zombie
    waitpid(info->si_pid, NULL, 0);
}

void rt_handler(int signal, siginfo_t *info, void *ucontext) {
    assert(signal >= SIGRTMIN && signal <= SIGRTMAX);
    if(PRINT_SIGRT) printf("Received SIGRT%d from %d\n", signal - SIGRTMIN, info->si_pid);
}

void sigint_handler(int signal, siginfo_t *info, void *ucontext) {
    for(int i = 0; i < N; ++i){
        if(children[i] != 0){
            kill(children[i], SIGKILL);
        }
    }
    exit(1);
}
