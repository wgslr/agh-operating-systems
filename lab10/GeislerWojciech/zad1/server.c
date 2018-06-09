// Wojciech Geisler
// 2018-06

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "common.h"

int open_network_socket(const short port) {
    const struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port =  htons(port),
            .sin_addr.s_addr = INADDR_ANY
    };

    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    OK(fd, "Error opening inet socket");
    OK(bind(fd, (const struct sockaddr *) &addr, sizeof(addr)), "Error binding inet socket")

    return fd;
}

int open_local_socket(const char *path) {
    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
    };
    strncpy(addr.sun_path, path, UNIX_PATH_MAX);

    const int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    OK(fd, "Error opening unix socket");
    OK(bind(fd, (struct sockaddr *) &addr, sizeof(addr)), "Error binding unix socket")

    return fd;

}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Incorrect number of arguments (expected 2)");
        exit(1);
    }

    short port = (short) atoi(argv[1]);

    open_local_socket(argv[2]);
    open_network_socket(port);

    sleep(10);

    return 0;
}