#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#define _XOPEN_SOURCE
#include <signal.h>

pid_t dater_pid = 0;

pid_t spawn_dater(void);

void handler(int signal){
    if(signal == SIGTSTP){
        if(dater_pid == 0) {
            dater_pid = spawn_dater();
        } else {
            kill(dater_pid, SIGKILL);
            // clear zombie
            waitpid(dater_pid, NULL, 0);
            dater_pid = 0;
            printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
        }
    } else if(signal == SIGINT) {
        printf("Odebrano sygnał SIGINT");
        kill(dater_pid, SIGKILL);
        exit(0);
    } else {
        fprintf(stderr, "Signal handler in pid %d received unexpected signal %d\n", getpid(), signal);
    }
}


pid_t spawn_dater(void) {
    int pid = fork();
    if(pid == 0) {
        int result = execlp("./dater.sh", "dater.sh", NULL);
        fprintf(stderr, "Could not spawn date-printing script: %d\n", result);
        exit(1);
    } else {
        return pid;
    }
}


int main(void) {

    signal(SIGINT, &handler);

    struct sigaction *sg = calloc(1, sizeof(struct sigaction));
    sg->sa_handler = &handler;
    sigaction(SIGTSTP, sg, NULL);

    dater_pid = spawn_dater();

    while(true) {}
}

