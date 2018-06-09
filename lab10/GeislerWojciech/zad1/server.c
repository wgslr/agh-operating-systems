// Wojciech Geisler
// 2018-06

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "common.h"

typedef struct {
    char name[MAX_NAME];
    int socket;
} client;


int unix_socket;
int inet_socket;

client clients[MAX_CLIENTS];

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


void network_handler(void) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    OK(accept(inet_socket, (struct sockaddr *) &addr, &addrlen), "Error accepting inet connection");


}


pthread_t spawn_network_handler(void) {
    pthread_attr_t *attr = calloc(1, sizeof(pthread_attr_t));
    pthread_attr_init(attr);
    pthread_t tid;

    OK(pthread_create(&tid, attr, (void *(*)(void *)) &network_handler, NULL), "Error creating network handler thread");
    return tid;
}


void read_input() {

}


int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Incorrect number of arguments (expected 2)");
        exit(1);
    }

    const short port = (short) atoi(argv[1]);

    unix_socket = open_local_socket(argv[2]);
    inet_socket = open_network_socket(port);

    spawn_network_handler();

    pause();

    return 0;
}