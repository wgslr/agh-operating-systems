// Wojciech Geisler
// 2018-04

#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include "common.h"
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

int client_queues[MAX_CLIENTS];
int client_idx = 0;

int server_queue = -1;

void onexit(void);

char *get_date(void) ;

void sigint_handler(int signal) ;

void create_queue(void) {
    // REMVOVE
    onexit();

    const char *home = getenv("HOME");
    int result = msgget(ftok(home, FTOK_PROJ_ID), IPC_CREAT | 0644u);
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

void sigint_handler(int signal) {
    assert(signal == SIGINT);
    exit(0);
}

void send_to(int client_id, msgbuf *msg) {
    int qid = client_queues[client_id];
    if(msgsnd(qid, msg, MSG_SZ, 0) < 0) {
        fprintf(stderr, "Sending message (type %ld) to %d (qid %d) failed: %d %s\n", msg->mtype, client_id, qid, errno,
                strerror(errno));
        exit(1);
    }
}

void handle_register(msgbuf *msg) {
    assert(msg->sender_id == -1);
    int client_msqid = *(int *) msg->content;

    fprintf(stderr, "Registering client qeueue %d\n", client_msqid);
    int id = client_idx++;
    client_queues[id] = client_msqid;

    // send message with client_msqid identifier
    msgbuf response = {0};
    response.sender_pid = getpid();
    response.mtype = msg->sender_pid;
    response.sender_id = SERVER_ID;
    *(int *) response.content = id;
    send_to(id, &response);
}

void handle_mirror(msgbuf *msg) {
    int size = (int) strlen(msg->content);

    msgbuf response = {0};
    response.sender_pid = getpid();
    response.mtype = msg->sender_pid;
    response.sender_id = SERVER_ID;
    for(int i = 0; i < size; ++i) {
        response.content[i] = msg->content[size - i - 1];
    }
    send_to(msg->sender_id, &response);
}

void handle_calc(msgbuf *msg) {
    arith_req req = *(arith_req *) msg->content;
    int result;
    switch(req.op) {
        case ADD:
            result = req.arg1 + req.arg2;
            break;
        case SUB:
            result = req.arg1 - req.arg2;
            break;
        case MUL:
            result = req.arg1 * req.arg2;
            break;
        case DIV:
            result = req.arg1 / req.arg2;
            break;
        default:
            fprintf(stderr, "Unknown arithmetic operation requested by %d\n", msg->sender_id);
            result = 0;
    }
    msgbuf response = {0};
    response.sender_pid = getpid();
    response.mtype = msg->sender_pid;
    response.sender_id = SERVER_ID;
    *(int *) response.content = result;
    send_to(msg->sender_id, &response);
}

void handle_time(msgbuf *msg) {
    char *date = get_date();

    msgbuf response = {0};
    response.sender_pid = getpid();
    response.mtype = msg->sender_pid;
    response.sender_id = SERVER_ID;
    strcpy(response.content, date);
    send_to(msg->sender_id, &response);
    free(date);
}

void receive_loop(void) {
    msgbuf *buff = calloc(1, sizeof(msgbuf));
    ssize_t read;
    bool should_end = false;

    fprintf(stderr, "Waiting for message\n");
    while((read = msgrcv(server_queue, buff, MSG_SZ, 0, should_end ? IPC_NOWAIT : 0)) >= 0) {
        assert(read > 0);
        assert(read <= (ssize_t) MSG_SZ);

        switch(buff->mtype) {
            case REGISTER:
                fprintf(stderr, "Handling REGISTER from %d\n", buff->sender_pid);
                handle_register(buff);
                break;
            case MIRROR:
                fprintf(stderr, "Handling MIRROR from %d\n", buff->sender_pid);
                handle_mirror(buff);
                break;
            case CALC:
                fprintf(stderr, "Handling CALC from %d\n", buff->sender_pid);
                handle_calc(buff);
                break;
            case TIME:
                fprintf(stderr, "Handling TIME from %d\n", buff->sender_pid);
                handle_time(buff);
                break;
            case END:
                fprintf(stderr, "Handling END from %d\n", buff->sender_pid);
                should_end = true;
                break;
            case STOP:
                fprintf(stderr, "Handling STOP from %d\n", buff->sender_pid);
                msgctl(client_queues[buff->sender_id], IPC_RMID, NULL);
                client_queues[buff->sender_id] = -1;
                break;
            default:
                fprintf(stderr, "Server received unexpected message of type %ld", buff->mtype);
        }
    }
}

int main(void) {
    create_queue();
    atexit(&onexit);
    signal(SIGINT, &sigint_handler);

    receive_loop();

    return 0;
}

char *get_date(void) {
    FILE *pipe = popen("date", "r");

    char *line = NULL;
    size_t length = 0;
    if(getline(&line, &length, pipe) == -1) {
        fprintf(stderr, "Error reading date pipe: %s\n", strerror(errno));
        exit(1);
    }
    pclose(pipe);
    return line;
}

