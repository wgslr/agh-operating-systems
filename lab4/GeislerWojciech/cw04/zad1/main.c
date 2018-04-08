#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

pid_t spawn_dater() {
    int pid = fork();
    if(pid == 0) {
        execl("dater.sh", "dater.sh");
    } else {
        return pid;
    }
}


void child() {

}



int main() {

}

