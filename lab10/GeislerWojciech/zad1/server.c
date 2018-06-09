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
    char name[MAX_NAME];
    int socket;
} client;


int unix_socket;
int inet_socket;
int client_count;

client clients[MAX_CLIENTS] = {0};

void process_message(const message *msg, int socket) ;

void handle_register(const char *name, int socket) ;

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

    fprintf(stderr, "New connection accepted\n");

    if(client_count >= MAX_CLIENTS) {
        fprintf(stderr, "Cannot add client, maximum number reached");
    }


//    // find free place in clients array
//    int i = 0;
//    while(i < MAX_CLIENTS && clients[i].socket == 0)
//        ++i;
//
//    assert(i < MAX_CLIENTS);
//
//    clients[i].socket = client_fd;
//
//    // register client socket in fd
    struct epoll_event event = {
            .events = EPOLLIN | EPOLLPRI,
            .data.fd = client_fd
    };
    OK(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event), "Could not add client socket to epoll");

    fprintf(stderr, "New connection added to epoll\n");
}


void handle_message(int socket) {
    ssize_t bytes;
    message *buff = calloc(1, sizeof(message) + MAX_LEN);
    bytes = recv(socket, buff, sizeof(message), 0);
    OK(bytes, "Error receiving message header")

    if(buff->len > 0) {
        bytes = recv(socket, &buff->data, buff->len, 0);
        OK(bytes, "Error receiving message body")
    }

    process_message(buff, socket);
}


void process_message(const message *msg, int socket) {
    switch(msg->type) {
        case REGISTER:
            fprintf(stderr, "Client %.*s registering\n", MAX_NAME, msg->client_name);
            handle_register(msg->client_name, socket);
            break;
        default:
            fprintf(stderr, "Unexpected message type: %d", msg->type);
    }
}


void send_message(int socket, msg_type type, void * data, size_t len) {
    message *msg = calloc(1, sizeof(message) + len);
    msg->type = type;
    msg->len = len;
    memcpy(msg->data, data, len);
    OK(send(socket, msg, sizeof(msg) + len, 0), "Error sending message");
}


void handle_register(const char *name, int socket) {
    int available_idx = -1;
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(clients[i].socket <= 0) {
            available_idx = i;
        } else if (strcmp(clients[i].name, name) == 0) {
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


void network_handler(void) {
    fprintf(stderr, "network handler says hello\n");

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
            handle_message(event_socket);
        }
    }

//
//    printf("Accept()\n");
//
//
//    int client_fd = accept(inet_socket, (struct sockaddr *) &addr, &addrlen);
//    OK(client_fd, "Error accepting inet connection");
//
//    printf("Accepted new connection\n");
//
//    message h;
//    char data[100];
//    ssize_t bytes;
//    // accept one register
//    bytes = recv(client_fd, &h, sizeof(h), 0);
////    OK(bytes, "Error in recv")
//    printf("received %zd bytes\n", bytes);
//
//    if(h.type == REGISTER) {
//        printf("Client %.*s registering\n", MAX_NAME, h.client_name);
//    }
//
//    recv(client_fd, &data, h.len > 100 ? 100 : h.len, 0);
//    OK(bytes, "Error in recv")
//    printf("received %zd bytes\n", bytes);

}



pthread_t spawn(void *(* func) (void *)) {
    pthread_attr_t *attr = calloc(1, sizeof(pthread_attr_t));
    pthread_attr_init(attr);
    pthread_t tid;

    OK(pthread_create(&tid, attr, func, NULL), "Error creating network handler thread");
    return tid;
}


int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Incorrect number of arguments (expected 2)");
        exit(1);
    }

    const short port = (short) atoi(argv[1]);
    client_count = 0;

    unix_socket = open_local_socket(argv[2]);
    inet_socket = open_network_socket(port);

    spawn((void *(*)(void *)) &network_handler);

    pause();

    return 0;
}