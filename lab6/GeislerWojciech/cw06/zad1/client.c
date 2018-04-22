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
#include <memory.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_TOKENS 5
const char *const WHITESPACE = " \r\n\t";

typedef struct {
    char *toks[MAX_TOKENS + 1];
    int size;
} tokens;

int client_id = -1;
int client_queue;
int server_queue;

// Splits string on whitespace
tokens *tokenize(char *string) ;

void send(msgbuf *req) {
    OK(msgsnd(server_queue, req, MSG_SZ, 0), "Error sending message");
}

msgbuf *receive(void) {
    msgbuf *buff = calloc(1, sizeof(msgbuf));
    msgrcv(client_queue, buff, MSG_SZ, 0, 0);
    return buff;
}

void register_client(void) {
    msgbuf req = {
            .sender_id = client_id, .sender_pid = getpid(),
            .mtype = REGISTER,
            .content = {0}
    };
    *(int *) req.content = client_queue;
    send(&req);

    msgbuf *resp = receive();
    client_id = *(int *) resp->content;
    printf("Client id is %d\n", client_id);
    free(resp);
}

void handle_mirror(char *message) {
    msgbuf req = {
            .sender_id = client_id, .sender_pid = getpid(),
            .mtype = MIRROR,
            .content = {0}
    };
    strcpy(req.content, message);
    send(&req);

    printf("Mirroring. Clien id %d\n", client_id);
    msgbuf *resp = receive();
    printf("Mirrored: '%s'\n", resp->content);
    free(resp);
}

int main(int argc, char *argv[]) {
    FILE *input;
    if(argc <= 1 || strcmp("-", argv[1]) == 0) {
        fprintf(stderr, "Reading input from stdin\n");
        input = stdin;
    } else {
        input = fopen(argv[1], "r");
        if(input == NULL) {
            fprintf(stderr, "Opening input file '%s' failed: %s\n", argv[1], strerror(errno));
        }
    }

    const char *home = getenv("HOME");
    server_queue = msgget(ftok(home, FTOK_PROJ_ID), 0644u);
    OK(server_queue, "Opening server queue failed");
    client_queue = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0644u);
    OK(client_queue, "Creating client queue failed");

    register_client();

    char *line = NULL;
    size_t length = 0;
    while(getline(&line, &length, input)) {
        fprintf(stderr, "Handling line '%s'\n", line);

        tokens *command = tokenize(line);
        if(strcmp(command->toks[0], "MIRROR") == 0) {
            if(command->size < 2) {
                fprintf(stderr, "You must provider string to be mirrored");
            } else {
                handle_mirror(command->toks[1]);
            }
        }
    }
    free(line);


//    char *mirror = "Bede lustrem";
//    to_send->mtype = MIRROR;
//    to_send->sender_id = client_id;
//    strcpy(to_send->content, mirror);
//    OK(msgsnd(server_queue, to_send, MSG_SZ, 0), "Error sending mirror message");
//
//    read = msgrcv(client_queue, buff, MSG_SZ, 0, 0);
//    assert(read > 0);
//    assert(buff->sender_id == SERVER_ID);
//    assert(buff->mtype == MIRROR_RESP);
//
//    printf("Mirrored: %s\n", buff->content);
//
//    arith_req calc_arg = {.op = MUL, .arg1 = 3, .arg2 = 4};
//    to_send->mtype = CALC;
//    memcpy(to_send->content, &calc_arg, sizeof(calc_arg));
//
//    OK(msgsnd(server_queue, to_send, MSG_SZ, 0), "Error sending calc message");
//
//    read = msgrcv(client_queue, buff, MSG_SZ, 0, 0);
//    assert(read > 0);
//    assert(buff->sender_id == SERVER_ID);
//    assert(buff->mtype == CALC_RESP);
//
//    printf("Calced: %d\n", *(int *) buff->content);
//
//    to_send->mtype = TIME;
//
//    OK(msgsnd(server_queue, to_send, MSG_SZ, 0), "Error sending calc message");
//
//    read = msgrcv(client_queue, buff, MSG_SZ, 0, 0);
//    assert(read > 0);
//    assert(buff->sender_id == SERVER_ID);
//    assert(buff->mtype == TIME_RESP);
//
//    printf("Time is: %s\n", buff->content);
//
//    to_send->mtype = END;

//    OK(msgsnd(server_queue, to_send, MSG_SZ, 0), "Error sending calc message");
    return 0;
}

// Splits string on whitespace
tokens *tokenize(char *string) {
    tokens *result = calloc(1, sizeof(tokens));
    size_t wordlen;

    // find beginning of string
    string += strspn(string, WHITESPACE);

    while(string != NULL && *string != '\0') {
        wordlen = strcspn(string, WHITESPACE);
        result->toks[result->size] = calloc(1, wordlen);
        strncpy(result->toks[result->size], string, wordlen);
        ++result->size;

        string += wordlen;
        string += strspn(string, WHITESPACE); // skip whitespace
    }
    return result;
}
