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

int L;
int Type;

int sent = 0;
int received = 0;
pid_t child;
bool confirmed = false;

pid_t spawn_child(void);

void handler(int signal);

void set_handlers(void) ;

void int_handler(int signal) ;

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Number of signals L and sending method Type must be given");
        return 1;
    }
    L = atoi(argv[1]);
    Type = atoi(argv[2]);

    set_handlers();

    int sig1 = Type == 3 ? SIGRTMIN + 1 : SIGUSR1;
    int sig2 = Type == 3 ? SIGRTMIN + 2 : SIGUSR2;

    child = spawn_child();

    sleep(1);

    for(int i = 0; i < L; ++i){
        printf("Sending %dnth signal %d to child\n", ++sent, sig1);

        confirmed = false;
        kill(child, sig1);

        if(Type == 2){
            while(!confirmed) {}
        }
    }
    printf("Sent all signals\n");

    sleep(1);
    kill(child, sig2);
    waitpid(child, NULL, WNOHANG);

    printf("Parent summary:\n%4d signals sent\n%4d signals received\n", sent, received);
}


void set_handlers(void) {
    struct sigaction *sg = calloc(1, sizeof(struct sigaction));

    sg->sa_flags = 0;//| SA_NODEFER;
    sg->sa_handler = &handler;
    sigaction(SIGUSR1, sg, NULL);
    sigaction(SIGRTMIN + 1, sg, NULL);

    sg->sa_handler = &int_handler;
    sigaction(SIGINT, sg, NULL);

    free(sg);
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

void handler(int signal) {
    confirmed = true;
    ++received;
    printf("Parent received %dnth signal %d\n", received, signal);
}

void int_handler(int signal) {
    (void) signal; // unused
    kill(child, Type == 3 ? SIGRTMIN + 2 : SIGUSR2);
    exit(1);
}
