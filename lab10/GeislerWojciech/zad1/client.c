// Wojciech Geisler
// 2018-06

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include "common.h"

bool use_inet;

bool connection_type(char *arg) {
    if(strcmp(arg, "UNIX") == 0) {
        return false;
    } else if (strcmp(arg, "INET") == 0) {
        return true;
    } else {
        fprintf(stderr, "Invalid socket type. Expected UNIX or INET");
        exit(1);
    }
}


//int open_network_socket() {
//    const int fd =
//    OK(fd, "Error opening inet socket");
//    return fd;
//}


int open_local_socket(const char *path) {
    struct sockaddr_un addr = {
            .sun_family = AF_UNIX,
    };
    strncpy(addr.sun_path, path, UNIX_PATH_MAX);

    const int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    OK(fd, "Error opening unix socket");
    return fd;
}

void connect_inet(int fd, const char * addr_str) {
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


void connect_local(int fd, const char * path) {
    struct sockaddr_un sockaddr = {
            .sun_family = AF_UNIX,
    };
    strncpy(sockaddr.sun_path, path, UNIX_PATH_MAX);

    OK(connect(fd, (const struct sockaddr *) &sockaddr, sizeof(sockaddr)), "Error connecting to inet socket");

    fprintf(stderr, "Connected to %s\n", path);
}

void do_register(int fd, const char * name) {
    header head = {
            .type = REGISTER,
            .len = 0
    };
    strncpy(head.client_name, name, MAX_NAME);

    fprintf(stderr, "Registering\n");

    ssize_t sent = send(fd, &head, sizeof(head), 0);
    OK(sent, "Error sending header");
    printf("Sent %zd bytes", sent);

}


int main(int argc, char *argv[]) {
    if(argc != 4) {
        fprintf(stderr, "Incorrect number of arguments (expected 2)");
        exit(1);
    }

    int socketfd;
    char name[MAX_NAME];
    char path[UNIX_PATH_MAX];

    strncpy(name, argv[1], MAX_NAME);
    strncpy(path, argv[3], UNIX_PATH_MAX);

    use_inet = connection_type(argv[2]);

    if(use_inet) {
        socketfd = socket(AF_INET, SOCK_STREAM, 0);;
    } else {
        strncpy(path, argv[3], UNIX_PATH_MAX);
        socketfd = socket(AF_UNIX, SOCK_STREAM, 0);;
    };
    OK(socketfd, "Error opening socket")

    if(use_inet) {
        connect_inet(socketfd, path);
    } else {
        connect_local(socketfd, path);
    }

    do_register(socketfd, name);

    return 0;
}
