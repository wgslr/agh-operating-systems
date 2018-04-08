#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define _XOPEN_SOURCE
#include <signal.h>

bool print = true;

void handler(int signal){
    if(signal == SIGTSTP){
        if(print) {
            print = false;
            printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
        } else {
            print = true;
        }
    } else if(signal == SIGINT) {
        printf("Odebrano sygnał SIGINT");
        exit(0);
    } else {
        fprintf(stderr, "Signal handler in pid %d received unexpected signal %d\n", getpid(), signal);
    }
}

pid_t spawn_dater(void) {
    int pid = fork();
    if(pid == 0) {
        int result = execl("dater.sh", "dater.sh", NULL);
        fprintf(stderr, "Could not spawn date-printing script: %d\n", result);
        exit(1);
    } else {
        return pid;
    }
}

void print_date(void) {
    time_t timer;
    char buffer[10];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 10, "%H:%M:%S", tm_info);
    printf("%s\n", buffer);
}


int main(void) {

    signal(SIGINT, &handler);

    struct sigaction *sg = calloc(1, sizeof(struct sigaction));
    sg->sa_handler = &handler;
    sigaction(SIGTSTP, sg, NULL);

    while(true) {
        if(print) {
            print_date();
        }
    }
}

