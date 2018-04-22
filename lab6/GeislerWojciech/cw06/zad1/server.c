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

void onexit(void) ;

void create_queue(void) {
    // REMVOVE
    onexit();

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

void send_to(int client_id, msgbuf *msg) {
    int qid = client_queues[client_id];
    if(msgsnd(qid, msg, MSG_SZ, 0) < 0) {
        fprintf(stderr, "Sending message (type %ld) to %d (qid %d) failed: %d %s\n", msg->mtype, client_id, qid, errno, strerror(errno));
        exit(1);
    }
}

void handle_register(msgbuf *msg) {
    assert(msg->sender_id == -1);
    int client_msqid = *(int*) msg->content;

    fprintf(stderr, "Registering client qeueue %d\n", client_msqid);
    int id = client_idx++;
    client_queues[id] = client_msqid;

    // send message with client_msqid identifier
    msgbuf response = {0};
    response.mtype = REGISTER_ACK;
    response.sender_id = SERVER_ID;
    *(int*) response.content = id;
    send_to(id, &response);
}

void receive_loop(void) {
//    struct msgbuf *buff = calloc(1, sizeof(long) + sizeof(clientmsg));
    msgbuf *buff = calloc(1, sizeof(msgbuf));
    ssize_t read;

    fprintf(stderr, "Waiting for message\n");
    while((read = msgrcv(server_queue, buff, MSG_SZ, 0, 0)) >= 0) {
        assert(read > 0);
        assert(read <= (ssize_t) MSG_SZ);

        long mtype = buff->mtype;
        int sender_id = buff->sender_id;
        switch(mtype) {
            case REGISTER:
                handle_register(buff);
                break;
            default:
                fprintf(stderr, "Server received unexpected message of type %ld", mtype);
        }

    }
}

int main(int argc, char *argv[], char *env[]) {
    create_queue();
    atexit(&onexit);

    receive_loop();

    //TODO handle SIGINT

    return 0;
}