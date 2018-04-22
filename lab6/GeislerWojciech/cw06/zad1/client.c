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

int client_id = -1;

int main(void) {
    const char *home = getenv("HOME");
    int server_queue = msgget(ftok(home, FTOK_PROJ_ID), 0644u);
    OK(server_queue, "Opening server client_queue failed");

    int client_queue = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0644u);

    msgbuf *to_send = calloc(1, sizeof(msgbuf));
    to_send->mtype = REGISTER;
    to_send->sender_id = -1;
    *(int *) to_send->content = client_queue;

    fprintf(stderr, "Registering %d with server queue %d (size: %tu)\n", client_queue, server_queue, MSG_SZ);
    OK(msgsnd(server_queue, to_send, MSG_SZ, 0), "Error sending registration message");

    fprintf(stderr, "Sent registration message\n");

    msgbuf *buff = calloc(1, sizeof(msgbuf));
    ssize_t read;

    fprintf(stderr, "Waiting for message\n");
    read = msgrcv(client_queue, buff, MSG_SZ, 0, 0);
    assert(read > 0);
    assert(buff->sender_id == SERVER_ID);
    assert(buff->mtype == REGISTER_RESP);

    client_id = *(int *) buff->content;
    printf("Client id is %d\n", client_id);

    char *mirror = "Bede lustrem";
    to_send->mtype = MIRROR;
    to_send->sender_id = client_id;
    strcpy(to_send->content, mirror);
    OK(msgsnd(server_queue, to_send, MSG_SZ, 0), "Error sending mirror message");

    read = msgrcv(client_queue, buff, MSG_SZ, 0, 0);
    assert(read > 0);
    assert(buff->sender_id == SERVER_ID);
    assert(buff->mtype == MIRROR_RESP);

    printf("Mirrored: %s\n", buff->content);

}
