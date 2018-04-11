// Wojciech Geisler

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>

int received = 0;

void respond_handler(int signal) {
    (void) signal; // unused
    ++received;
//    printf("Child received %dnth signal %d\n", received, signal);
    kill(getppid(), signal);
}

void exit_handler(int signal) {
    (void) signal; // unused
    printf("Child received exit signal %d\n", signal);
    printf("Total of %d signals was handled by child", received);
    exit(0);
}

int main(void) {
    sigset_t mask;
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGRTMIN + 2);

    struct sigaction *sg = calloc(1, sizeof(struct sigaction));
    sg->sa_flags = SA_NODEFER;
    sg->sa_mask = mask;
    sg->sa_handler = &respond_handler;
    sigaction(SIGUSR1, sg, NULL);

//    sigaddset(&mask, SIGRTMIN + 1);
    sg->sa_mask = mask;
    signal(SIGRTMIN + 1, &respond_handler);

    sigemptyset(&mask);
    sg->sa_mask = mask;
    signal(SIGUSR2, &exit_handler);
    signal(SIGRTMIN + 2, &exit_handler);

    while(true) {}
}
