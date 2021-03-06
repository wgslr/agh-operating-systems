// Wojciech Geisler
// 2018-04

#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include "common.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>

mqd_t client_queues[MAX_CLIENTS] = {0};
int client_idx = 0;

mqd_t server_queue = -1;

void onexit(void);

char *get_date(void) ;

void sigint_handler(int signal) ;

void create_queue(void) {
    struct mq_attr attr = {
            .mq_flags = 0, .mq_maxmsg = 10, .mq_curmsgs = 0, .mq_msgsize = MSG_SZ
    };
    server_queue = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT | O_EXCL, 0644, &attr);

    if(server_queue < 0) {
        fprintf(stderr, "Error creating server queue: %s\n", strerror(errno));
        exit(1);
    }
}

void onexit(void) {
    fprintf(stderr, "Removing queue %d\n", server_queue);
    mq_unlink(SERVER_QUEUE);
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(client_queues[i] > 0) {
            fprintf(stderr, "Closing queue with descriptor %d\n", client_queues[i]);
            mq_close(client_queues[i]);
        }
    }
}

void sigint_handler(int signal) {
    assert(signal == SIGINT);
    exit(0);
}

void send_to(int client_id, msgbuf *msg) {
    mqd_t qid = client_queues[client_id];
    if(mq_send(qid, (char*) msg, MSG_SZ, PRIORITY) < 0) {
        fprintf(stderr, "Sending message to %d (qid %d) failed: %d %s\n", client_id, qid, errno, strerror(errno));
        exit(1);
    }
}

void handle_register(msgbuf *msg) {
    assert(msg->sender_id == -1);
    char* client_queue = msg->content;

    fprintf(stderr, "Registering client qeueue %s\n", client_queue);

    int id = client_idx;
    client_queues[id] = mq_open(client_queue, O_WRONLY);
    ++client_idx;

    // send message with client_msqid identifier
    msgbuf response = {0};
    response.sender_pid = getpid();
    response.mtype = REGISTER_RESP;
    response.sender_id = SERVER_ID;
    *(int *) response.content = id;
    send_to(id, &response);
}

void handle_mirror(msgbuf *msg) {
    int size = (int) strlen(msg->content);

    msgbuf response = {0};
    response.sender_pid = getpid();
    response.mtype = MIRROR_RESP;
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
    response.mtype = CALC_RESP;
    response.sender_id = SERVER_ID;
    *(int *) response.content = result;
    send_to(msg->sender_id, &response);
}

void handle_time(msgbuf *msg) {
    char *date = get_date();

    msgbuf response = {0};
    response.sender_pid = getpid();
    response.mtype = TIME_RESP;
    response.sender_id = SERVER_ID;
    strcpy(response.content, date);
    send_to(msg->sender_id, &response);
    free(date);
}

void set_nonblock(void) {
    struct mq_attr *attr = calloc(1, sizeof(struct mq_attr));
    mq_getattr(server_queue, attr);
    attr->mq_flags |= O_NONBLOCK;
    mq_setattr(server_queue, attr, NULL);
}

void receive_loop(void) {
    msgbuf *buff = calloc(1, sizeof(msgbuf));
    ssize_t read;
    unsigned priority;

    fprintf(stderr, "Waiting for message\n");
    while((read = mq_receive(server_queue, (char*)buff, MSG_SZ, &priority)) >= 0) {
        assert(priority == PRIORITY);
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
                set_nonblock();
                break;
            case STOP:
                fprintf(stderr, "Handling STOP from %d\n", buff->sender_pid);
                mq_close(client_queues[buff->sender_id]);
                client_queues[buff->sender_id] = -1;
                break;
            default:
                fprintf(stderr, "Server received unexpected message of type %d", buff->mtype);
        }
    }
    fprintf(stderr, "mq_receive returned %td: %d %s\n", read, errno, strerror(errno));
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

