// Wojciech Geisler
// 2018-06

#include <stdio.h>
#include <stdbool.h>
#include "common.h"

bool use_inet;
int socketfd;

bool connection_type(char *arg) {
    if(strcmp(arg, "UNIX")) {
        return false;
    } else if (strcmp(arg, "INET")) {
        return true;
    } else {
        fprintf(stderr, "Invalid socket type. Expected UNIX or INET");
        exit(1);
    }
}


int open_network_socket(const short port) {
    const struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port =  htons(port),
            .sin_addr.s_addr = INADDR_ANY
    };

    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    OK(fd, "Error opening inet socket");
    return fd;
}


int open_local_socket(const char *path) {
    struct sockaddr_un addr = {
            .sun_family = AF_UNIX,
    };
    strncpy(addr.sun_path, path, UNIX_PATH_MAX);

    const int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    OK(fd, "Error opening unix socket");
    return fd;
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        fprintf(stderr, "Incorrect number of arguments (expected 2)");
        exit(1);
    }

    char name[MAX_NAME];
    char path[UNIX_PATH_MAX] = "";
    short port = -1;

    use_inet = connection_type(argv[2]);

    if(use_inet == AF_UNIX) {
        port = (short) atoi(argv[3]);
        socketfd = open_network_socket(port);
    } else {
        strncpy(path, argv[3], UNIX_PATH_MAX);
        socketfd = open_local_socket(path);
    };


    return 0;
}
