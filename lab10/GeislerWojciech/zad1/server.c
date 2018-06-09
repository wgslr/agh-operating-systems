// Wojciech Geisler
// 2018-06

#include <sys/epoll.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <assert.h>
#include <hdf5.h>
#include "common.h"

typedef struct {
    char name[MAX_NAME + 1];
    int socket;
} client;

typedef struct {
    short port;
    char path[UNIX_PATH_MAX];
} listener_args;


int unix_socket;
int inet_socket;
int client_count;

client clients[MAX_CLIENTS] = {0};

void process_message(const message *msg, int socket);

void handle_register(const char *name, int socket);

void cleanup(void);

int open_local_socket(const char *path);

int open_network_socket(const short port);

message *read_message(int socket);

void accept_connection(int socket, int epoll_fd);

// Spawns a thread with default attributes
pthread_t spawn(void *(*func)(void *), void *args);

void *listener(void *arg);

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Incorrect number of arguments (expected 2)");
        exit(1);
    }

    atexit(&cleanup);
    client_count = 0;

    listener_args *args = calloc(1, sizeof(listener_args));
    args->port = (short) atoi(argv[1]);
    strncpy(args->path, argv[3], UNIX_PATH_MAX);


    spawn((void *(*)(void *)) &listener, (void *) args);

    pause();

    return 0;
}

/*********************************************************************************
* Socket listener
*********************************************************************************/

void *listener(void *arg) {
    listener_args *args = (listener_args *) arg;
    unix_socket = open_local_socket(args->path);
    inet_socket = open_network_socket(args->port);

    int epoll_fd = epoll_create1(0);
    struct epoll_event event = {
            .events = EPOLLIN | EPOLLPRI,
            .data.fd = unix_socket
    };
    OK(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, unix_socket, &event), "Could not add local socket to epoll");

    event.data.fd = inet_socket;
    OK(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inet_socket, &event), "Could not add inet socket to epoll");

    while(true) {
        OK(epoll_wait(epoll_fd, &event, 1, -1), "Error waiting for message");

        int event_socket = event.data.fd;
        if(event_socket == inet_socket || event_socket == unix_socket) {
            accept_connection(event_socket, epoll_fd);
        } else {
            message *msg = read_message(event_socket);
            process_message(msg, event_socket);
        }
    }
    return NULL;
}

int open_network_socket(const short port) {
    const struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port =  htons(port),
            .sin_addr.s_addr = INADDR_ANY
    };

    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    OK(fd, "Error opening inet socket");
    OK(bind(fd, (const struct sockaddr *) &addr, sizeof(addr)), "Error binding inet socket");
    OK(listen(fd, MAX_CLIENTS), "Error listening on inet socket");

    return fd;
}

int open_local_socket(const char *path) {
    struct sockaddr_un addr = {
            .sun_family = AF_UNIX,
    };
    strncpy(addr.sun_path, path, UNIX_PATH_MAX);

    const int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    OK(fd, "Error opening unix socket");
    OK(bind(fd, (struct sockaddr *) &addr, sizeof(addr)), "Error binding unix socket");
    OK(listen(fd, MAX_CLIENTS), "Error listening on local socket");

    return fd;
}

void accept_connection(int socket, int epoll_fd) {
    int client_fd = accept(socket, NULL, NULL);
    OK(client_fd, "Error accepting connection");

    fprintf(stderr, "New connection accepted as socket %d\n", client_fd);

    if(client_count >= MAX_CLIENTS) {
        fprintf(stderr, "Cannot add client, maximum number reached");
    }


    struct epoll_event event = {
            .events = EPOLLIN,
            .data.fd = client_fd
    };
    OK(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event), "Could not add client socket to epoll");

    fprintf(stderr, "New connection added to epoll\n");
}

message *read_message(int socket) {
    ssize_t bytes;
    message *buff = calloc(1, sizeof(message) + MAX_LEN);
    bytes = recv(socket, buff, sizeof(message), 0);
    OK(bytes, "Error receiving message header")

    if(bytes == 0) {
        fprintf(stderr, "Client closed connection\n");
        shutdown(socket, SHUT_RDWR);
        close(socket);
        return NULL;
    }

    if(buff->len > 0) {
        bytes = recv(socket, &buff->data, buff->len, 0);
        OK(bytes, "Error receiving message body")
    }

    return buff;
}

void send_message(int socket, msg_type type, void *data, size_t len) {
    message *msg = calloc(1, sizeof(message) + len);
    msg->type = type;
    msg->len = len;
    memcpy(msg->data, data, len);
    OK(send(socket, msg, sizeof(msg) + len, 0), "Error sending message");
    free(msg);
}


/*********************************************************************************
* Application-logic functions
*********************************************************************************/


void process_message(const message *msg, int socket) {
    switch(msg->type) {
        case REGISTER:
            fprintf(stderr, "Client %s registering\n", msg->client_name);
            handle_register(msg->client_name, socket);
            break;
        default:
            fprintf(stderr, "Unexpected message type: %d at socket %d\n", msg->type, socket);
    }
}


void handle_register(const char *name, int socket) {
    int available_idx = -1;
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(clients[i].socket <= 0) {
            available_idx = i;
        } else if(strcmp(clients[i].name, name) == 0) {
            // name exists
            printf("Refusing registration of duplicate name '%.*s'\n", MAX_NAME, name);
            send_message(socket, NAME_TAKEN, NULL, 0);
            return;
        }
    }

    strncpy(clients[available_idx].name, name, MAX_NAME);
    clients[available_idx].socket = socket;

    printf("Registered client '%.*s'\n", MAX_NAME, name);
    send_message(socket, REGISTER_ACK, NULL, 0);
}


/*********************************************************************************
* Utils
*********************************************************************************/

// Spawns a thread with default attributes
pthread_t spawn(void *(*func)(void *), void *args) {
    pthread_attr_t *attr = calloc(1, sizeof(pthread_attr_t));
    pthread_attr_init(attr);
    pthread_t tid;

    OK(pthread_create(&tid, attr, func, args), "Error creating network handler thread");
    pthread_attr_destroy(attr);
    free(attr);
    return tid;
}


void cleanup(void) {
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(clients[i].socket > 0) {
            shutdown(clients[i].socket, SHUT_RDWR);
            close(clients[i].socket);
        }
    }

    shutdown(inet_socket, SHUT_RDWR);
    shutdown(unix_socket, SHUT_RDWR);
    close(inet_socket);
    close(unix_socket);
}