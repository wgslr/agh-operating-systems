// Wojciech Geisler
// 2018-04

#define _XOPEN_SOURCE

#include <stdio.h>
#include "common.h"
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>

int client_queues[MAX_CLIENTS];
int client_idx = 0;

int server_queue = -1;

void create_queue(void) {
    const char *home = getenv("HOME");
    int result = msgget(ftok(home, FTOK_PROJ_ID), IPC_CREAT |  0644u);
    if(result < 0) {
        fprintf(stderr, "Error creating server queue: %s\n", strerror(errno));
        exit(1);
    } else {
        fprintf(stderr, "Created queue %d\n", result);
        server_queue = result;
    }
}

void onexit(void) {
    fprintf(stderr, "Removing queue %d\n", server_queue);
    msgctl(server_queue, IPC_RMID, NULL);
}

void register_client(int queue) {
    fprintf(stderr, "Registering client qeueu %d\n", queue);
    int id = client_idx++;

    // send message with queue identifier
    client_queues[id] = queue;
    msgbuf msg = {0};
    msg.mtype = REGISTER_ACK;
    msg.sender_msqid = server_queue;
    *(int*) msg.content = id;
    OK(msgsnd(queue, &msg, sizeof(int), 0),
       "Sending client ID failed");
}

void handle_loop() {
//    struct msgbuf *buff = calloc(1, sizeof(long) + sizeof(clientmsg));
    msgbuf *buff = calloc(1, sizeof(msgbuf));
    ssize_t read;

    fprintf(stderr, "Waiting for message\n");
    while((read = msgrcv(server_queue, buff, MSG_SZ, 0, 0)) >= 0) {
        assert(read > 0);
        assert(read <= MSG_SZ);

        long mtype = buff->mtype;
        int client_msqid = buff->sender_msqid;
        switch(mtype) {
            case REGISTER:
                register_client(client_msqid);
                break;
            default:
                fprintf(stderr, "Server received unexpected message of type %ld", mtype);
        }

    }
}

int main(int argc, char *argv[], char *env[]) {
    create_queue();
    atexit(&onexit);

    handle_loop();

    //TODO handle SIGINT

    return 0;
}