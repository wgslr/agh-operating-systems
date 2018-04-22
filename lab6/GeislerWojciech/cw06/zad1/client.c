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

int main(void) {
    const char *home = getenv("HOME");
    int server_queue = msgget(ftok(home, FTOK_PROJ_ID), 0644u);
    OK(server_queue, "Opening server client_queue failed");

    int client_queue = msgget(IPC_PRIVATE, IPC_CREAT | 0644u);

    msgbuf *to_send = calloc(1, sizeof(msgbuf));
    to_send->mtype = REGISTER;
    to_send->sender_msqid = client_queue;
    memset(to_send->content, 0, CONTENT_SZ);

    fprintf(stderr, "Registering %d with server client_queue %d (size: %u)\n", client_queue, server_queue, MSG_SZ);
    OK(msgsnd(server_queue, to_send, MSG_SZ, 0), "Error sending registration message");

    fprintf(stderr, "Sent registration message\n");

}
