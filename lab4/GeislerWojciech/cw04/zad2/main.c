// Wojciech Geisler

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pid_t *children;
int request_count = 0;


pid_t spawn_child(void);

void spawn_children(int count);

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Number of children N and number of requests K must be given");
        return 1;
    }

    const int N = atoi(argv[1]);
//    const int K = atoi(argv[1]);
    spawn_children(N);
    sleep(10);


    return 0;
}


void spawn_children(int count) {
    children = calloc(count, sizeof(pid_t));
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

