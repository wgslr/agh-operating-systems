// Wojciech Geisler
// 2018-06

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

int calculate(arith_req *req) ;

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
    fprintf(stderr, "Splitting %s\n", addr_str);

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
    fprintf(stderr, "Connecting to 0x%08x:%u\n", addr.s_addr, port);

    OK(connect(fd, (const struct sockaddr *) &sockaddr, sizeof(sockaddr)), "Error connecting to inet socket");

    fprintf(stderr, "Connected to 0x%08X:%u\n", addr.s_addr, port);
}


void connect_local(int fd, const char *path) {
    struct sockaddr_un sockaddr = {
            .sun_family = AF_UNIX,
    };
    strncpy(sockaddr.sun_path, path, UNIX_PATH_MAX);

    OK(connect(fd, (const struct sockaddr *) &sockaddr, sizeof(sockaddr)), "Error connecting to inet socket");
    fprintf(stderr, "Connected to %s\n", path);
}

void send_message(int socket, msg_type type, void *data, size_t len) {
    message *msg = calloc(1, sizeof(message) + len);
    msg->type = type;
    msg->len = len;
    strncpy(msg->client_name, name, MAX_NAME);
    memcpy(msg->data, data, len);
    OK(send(socket, msg, sizeof(msg) + len, 0), "Error sending message");
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
        printf("Client '%s' registered succesfully\n", name);
    } else if(buff.type == NAME_TAKEN) {
        printf("Cannot register due to name conflict\n");
        exit(1);
    } else {
        fprintf(stderr, "Unexpected server response: %d\n", buff.type);
    }
}

void do_listen(int fd) {
    while(true) {
        message *buff = calloc(1, sizeof(message) + sizeof(arith_req));
        ssize_t bytes = recv(fd, buff, sizeof(message), 0);
        OK(bytes, "Error receiving message header");
        if(bytes == 0) {
            fprintf(stderr, "Server closed the connection\n");
            exit(1);
        }

        if(buff->type != ARITH) {
            fprintf(stderr, "Received unexpected message of type %d\n", buff->type);
            continue;
        }

        int result = calculate((arith_req *) buff->data);
        artih_resp resp = {
                .id = ((arith_req *) buff->data)->id,
                .result = result
        };
        send_message(fd, RESULT, &resp, sizeof(resp));
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
    shutdown(_socket_fd, SHUT_RDWR);
    close(_socket_fd);
}
