// Wojciech Geisler
// 2018-06

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <signal.h>
#include "common.h"

int _socket_fd;
char name[MAX_NAME + 1];

void cleanup(void);

int calculate(arith_req *req);

sa_family_t connection_type(char *arg) {
    if(strcmp(arg, "UNIX") == 0) {
        return AF_UNIX;
    } else if(strcmp(arg, "INET") == 0) {
        return AF_INET;
    } else {
        fprintf(stderr, "Invalid socket type. Expected UNIX or INET");
        exit(1);
    }
}


void connect_inet(int fd, const char *addr_str) {
    // split address and port
    char *delim = strstr(addr_str, ":");
    *delim = '\0';
    const unsigned short port = (unsigned short) atoi(delim + 1);

    struct in_addr addr;
    inet_aton(addr_str, &addr);

    struct sockaddr_in sockaddr = {
            .sin_family = AF_INET,
            .sin_addr = addr,
            .sin_port =  htons(port),
    };

    OK(connect(fd, (const struct sockaddr *) &sockaddr, sizeof(sockaddr)), "Error connecting to inet socket");
    fprintf(stderr, "Connected to server via INET\n");
}


void connect_local(int fd, const char *path) {
    struct sockaddr_un sockaddr = {
            .sun_family = AF_UNIX,
    };
    strncpy(sockaddr.sun_path, path, UNIX_PATH_MAX);

    OK(connect(fd, (const struct sockaddr *) &sockaddr, sizeof(sockaddr)), "Error connecting to inet socket");
    fprintf(stderr, "Connected to server via UNIX\n");
}

void send_message(int socket, msg_type type, void *data, size_t len) {
    message *msg = calloc(1, sizeof(message) + len);
    msg->type = type;
    msg->len = (uint32_t) len;
    strncpy(msg->client_name, name, MAX_NAME);
    if(len > 0) {
        memcpy(msg->data, data, len);
    }

    OK(send(socket, msg, sizeof(message) + len, 0), "Error sending message");
    free(msg);
}

void do_register(int fd) {
    message buff = {
            .type = REGISTER,
            .len = 0,
            .client_name = {0}
    };
    strncpy(buff.client_name, name, MAX_NAME);

    ssize_t sent = send(fd, &buff, sizeof(buff), 0);
    OK(sent, "Error sending message");

    // receive result
    recv(fd, &buff, sizeof(buff), 0);

    if(buff.type == REGISTER_ACK) {
        printf("Client '%s' registered successfully\n", name);
    } else if(buff.type == NAME_TAKEN) {
        printf("Cannot register due to name conflict\n");
        exit(1);
    } else {
        fprintf(stderr, "Unexpected server response: %d\n", buff.type);
    }
}

void do_listen(int fd) {
    const size_t LEN = sizeof(message) + sizeof(arith_req);
    message *buff = calloc(1, LEN);
    arith_req *req;
    while(true) {
        ssize_t bytes = recv(fd, buff, LEN, 0);
        OK(bytes, "Error receiving message header");
        if(bytes == 0) {
            fprintf(stderr, "Server closed the connection\n");
            exit(1);
        }

        switch(buff->type) {
            case PING:
                send_message(fd, PING, NULL, 0);
                break;
            case ARITH:
                req = (arith_req *) buff->data;
                fprintf(stderr, "Calculating #%d with %d and %d\n", req->id, req->arg1, req->arg2);

                int result = calculate(req);
                arith_resp resp = {
                        .id = req->id,
                        .result = result
                };

                send_message(fd, RESULT, &resp, sizeof(arith_resp));
                fprintf(stderr, "Sent response for #%d (%d)\n", req->id, result);
                break;
            default:
                fprintf(stderr, "Received unexpected message of type %d\n", buff->type);
                continue;
        }
    }
}

int calculate(arith_req *req) {
    int result;
    switch(req->op) {
        case ADD:
            result = req->arg1 + req->arg2;
            break;
        case SUB:
            result = req->arg1 - req->arg2;
            break;
        case MUL:
            result = req->arg1 * req->arg2;
            break;
        case DIV:
            result = req->arg1 / req->arg2;
            break;
        default:
            fprintf(stderr, "Unknown arithmetic operation\n");
            result = 0;
    }
    return result;
}

void sigint(int signo) {
    assert(signo == SIGINT);
    send_message(_socket_fd, UNREGISTER, NULL, 0);
}


int main(int argc, char *argv[]) {
    if(argc != 4) {
        fprintf(stderr, "Incorrect number of arguments (expected 2)");
        exit(1);
    }

    atexit(&cleanup);
    signal(SIGINT, &sigint);

    int socketfd;

    strncpy(name, argv[1], MAX_NAME);

    sa_family_t family = connection_type(argv[2]);

    socketfd = socket(family, SOCK_STREAM, 0);;
    OK(socketfd, "Error opening socket")
    _socket_fd = socketfd;

    if(family == AF_UNIX) {
        connect_local(socketfd, argv[3]);
    } else {
        connect_inet(socketfd, argv[3]);
    }

    do_register(socketfd);
    do_listen(socketfd);

    return 0;
}


void cleanup(void) {
    CHECK(shutdown(_socket_fd, SHUT_RDWR), "Shutdown error of client socket");
    CHECK(close(_socket_fd), "Error closing client socket");
    fprintf(stderr, "Cleanup finished, exiting\n");
}
