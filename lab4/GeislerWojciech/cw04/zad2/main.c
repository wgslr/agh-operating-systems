// Wojciech Geisler

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>

pid_t *children;
bool *allowed;
int request_count = 0;


pid_t spawn_child(void);

void spawn_children(int count);

void request_handler(int signal) {
    assert(signal == SIGUSR1);

    
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Number of children N and number of requests K must be given");
        return 1;
    }

    const int N = atoi(argv[1]);
//    const int K = atoi(argv[1]);
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

//    waitpid()
}

