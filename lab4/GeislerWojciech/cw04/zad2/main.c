// Wojciech Geisler

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#define _XOPEN_SOURCE
#include <signal.h>

int N, K;
pid_t *children;
bool *allowed;
int request_count = 0;


pid_t spawn_child(void);

void spawn_children(int count);

void send_allow(pid_t child);

void allow_all(void) ;

void request_handler(int signal, siginfo_t *info, void *ucontext) ;

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

    N = atoi(argv[1]);
    K = atoi(argv[1]);
    children = calloc(N, sizeof(pid_t));
    allowed = calloc(N, sizeof(bool));

    spawn_children(N);
    sleep(10);


    return 0;
}


void spawn_children(int count) {
    for(int i = 0; i < count; ++i) {
        children[i] = spawn_child();
    }
}

pid_t spawn_child(void) {
    pid_t pid = fork();
    if(pid == 0) {
        execl("./child", "child", NULL);
        exit(1);
    } else {
        return pid;
    }
}

void request_handler(int signal, siginfo_t *info, void *ucontext) {
    assert(signal == SIGUSR1);
    (void) ucontext; // unused

    pid_t caller = info->si_pid;
    int childnum = 0;
    while(childnum < N && children[childnum] != caller) ++childnum;

    if(!allowed[childnum]) {
        ++request_count;
        allowed[childnum] = true;
    }

    if(request_count == K) {
        allow_all();
    } else if(request_count > K) {
        send_allow(allowed[childnum]);
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
    kill(child, SIGALRM);
}

