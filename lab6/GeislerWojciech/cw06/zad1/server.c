// Wojciech Geisler
// 2018-04

#include <stdio.h>
#include "constants.h"
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>

int client_queues[MAX_CLIENTS];
int client_idx = 0;

#define OK(_EXPR, _ERR_MSG) if((_EXPR) < 0) { fprintf(stderr, _ERR_MSG); exit(1); }

int get_or_create_queue(void) {
    const char *home = getenv("HOME");
    int result = msgget(ftok(home, FTOK_PROJ_ID), IPC_CREAT | IPC_EXCL | 0644u);
    if(result < 0) {
        fprintf(stderr, "Error creating server queue: %s\n", strerror(errno));
        exit(1);
    }
    return result;
}

void onexit(void) {
    int queue = get_or_create_queue();
    msgctl(queue, IPC_RMID, NULL);
}

void register_client(int queue) {
    int id = client_idx++;
    client_queues[id] = queue;
    OK(msgsnd(queue, &id, sizeof(int), 0),
       "Sending client ID failed");
}

void handle_loop(int queue) {
    struct msgbuf *buff = calloc(1, sizeof(long) + sizeof(clientmsg));
    ssize_t read;
    while((read = msgrcv(queue, buff, MAX_MSGSZ, 0, 0)) >= 0) {
        assert(read > 0);

        long mtype = buff->mtype;
        clientmsg *msg = buff->mtext;
        int client = msg->sender;
        switch(mtype) {
            case REGISTER:
                register_client(client);

        }

    }
}

int main(int argc, char *argv[], char *env[]) {
    int queue = get_or_create_queue();
    atexit(&onexit);

    handle_loop(queue);

    return 0;
}