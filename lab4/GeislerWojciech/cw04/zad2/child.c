// Wojciech Geisler

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#define MAX_DELAY 10
int delay;

void handler(int signal) {
    int sigrt = SIGRTMIN + (rand() % (SIGRTMAX - SIGRTMIN + 1));
    kill(getppid(), sigrt);

    exit(delay);
}

int main(void) {
    srand(time(NULL) * getpid());
    delay = rand() % (MAX_DELAY + 1);

    signal(SIGALRM, &handler);

    sleep(delay);
    kill(getppid(), SIGUSR1);

    // wait for permission
    pause();
}

