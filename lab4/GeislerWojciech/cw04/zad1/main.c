#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

pid_t dater_pid = 0;

pid_t spawn_dater(void);
void child(void);

void child_handler(int signal){
    if(signal == SIGSTOP){
        if(dater_pid == 0) {
            dater_pid = spawn_dater();
        } else {
            kill(SIGKILL, dater_pid);
            dater_pid = 0;
        }
    } else if(signal == SIGINT) {
        printf("Odebrano sygnał SIGINT");
        exit(0);
    } else {
        fprintf(stderr, "pid %d received unexpected signal %d\n", getpid(), signal);
    }
}

void child(void) {
    dater_pid = spawn_dater();
    signal(SIGSTOP, &child_handler);

    // TODO use sigaction
    signal(SIGINT, &child_handler);
}

pid_t spawn_dater(void) {
    int pid = fork();
    if(pid == 0) {
        printf("Spawning dater\n");
        int result = execl("dater.sh", "dater.sh", NULL);
        printf("%d\n", result);
        exit(1);
    } else {
        return pid;
    }
}

pid_t spawn_child(void) {
    int pid = fork();
    if(pid == 0) {
        child();
        exit(0);
    } else {
        return pid;
    }
}


int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    pid_t child = spawn_child();
    sleep(5);
    printf("Sending SIGSTOP to child\n");
    kill(SIGSTOP, child);
    sleep(5);
    printf("Sending SIGSTOP to child\n");
    kill(SIGSTOP, child);
    sleep(5);
    printf("Sending SIGSTOP to child\n");
    kill(SIGSTOP, child);
    sleep(5);
    printf("Sending SIGINT to child\n");
    kill(SIGINT, child);

    return 0;
}

