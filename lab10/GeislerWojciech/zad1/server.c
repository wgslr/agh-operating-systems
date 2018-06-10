// Wojciech Geisler
// 2018-06

#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <sys/epoll.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>
#include "common.h"

typedef struct {
    char name[MAX_NAME + 1];
    int socket;
    bool responsive;
} client;

typedef struct {
    short port;
    char path[UNIX_PATH_MAX + 1];
} listener_args;


const char *const WHITESPACE = " \r\n\t";
typedef struct {
    int size;
    char *toks[MAX_TOKENS + 1];
} tokens;

int unix_socket;
int inet_socket;
char unix_socket_path[UNIX_PATH_MAX];
int client_count;
int op_id = 1;

client clients[MAX_CLIENTS] = {{{0}, 0, 0}};

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

arith_op char_to_op(char c);

client *get_random_client(void);

void *reader(void *);

// Splits string on whitespace
tokens *tokenize(char *string);

void handle_result(const arith_resp *resp, const char *client_name);

void handle_unregister(const char *name);

void handle_ping(int socket) ;

void *monitor(void *) ;

client *find_client_by_socket(const int socket) ;

void sigint(int signum) {
    (void) signum;
    exit(0);
}


int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Incorrect number of arguments (expected 2)");
        exit(1);
    }

    atexit(&cleanup);
    signal(SIGINT, &sigint);
    srand((unsigned int) time(NULL));
    client_count = 0;
    strncpy(unix_socket_path, argv[2], UNIX_PATH_MAX);

    listener_args *args = calloc(1, sizeof(listener_args));
    args->port = (short) atoi(argv[1]);
    strncpy(args->path, argv[2], UNIX_PATH_MAX);

    spawn(&listener, (void *) args);
    spawn((void *(*)(void *)) &reader, NULL);
    spawn((void *(*)(void *)) &monitor, NULL);

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

    fprintf(stderr, "Server listening at 0.0.0.0:%d and '%s'\n", args->port, args->path);

    while(true) {
        OK(epoll_wait(epoll_fd, &event, 1, -1), "Error waiting for message");

        int event_socket = event.data.fd;
        if(event_socket == inet_socket || event_socket == unix_socket) {
            accept_connection(event_socket, epoll_fd);
        } else {
            message *msg = read_message(event_socket);
            if(msg != NULL) {
                process_message(msg, event_socket);
            }
        }
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

    if(client_count >= MAX_CLIENTS) {
        fprintf(stderr, "Cannot add client, maximum number reached\n");
        return;
    }

    struct epoll_event event = {
            .events = EPOLLIN,
            .data.fd = client_fd
    };
    OK(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event), "Error adding connection to epoll");

    fprintf(stderr, "Accepted incoming connection\n");
}

message *read_message(int socket) {
    ssize_t bytes;
    message *buff = calloc(1, sizeof(message) + MAX_LEN);
    bytes = recv(socket, buff, sizeof(message), 0);
    OK(bytes, "Error receiving message header")

    if(bytes == 0) {
        client *c = find_client_by_socket(socket);
        if(c != NULL) {
            fprintf(stderr, "Client '%s' closed connection\n", c->name);
            handle_unregister(c->name);
        } else {
            fprintf(stderr, "Anonymous connection closed\n");
            shutdown(socket, SHUT_RDWR);
            close(socket);
        }
        return NULL;
    }

    if(buff->len > 0) {
        bytes = recv(socket, &buff->data, buff->len, 0);
        OK(bytes, "Error receiving message body")
        assert(bytes == (ssize_t) buff->len);
    }

    return buff;
}

void send_message(int socket, msg_type type, void *data, size_t len) {
    message *msg = calloc(1, sizeof(message) + len);
    msg->type = type;
    msg->len = len;
    if(len > 0) {
        memcpy(msg->data, data, len);
    }
    OK(send(socket, msg, sizeof(message) + len, 0), "Error sending message");
    free(msg);
}


/*********************************************************************************
* Message handler
*********************************************************************************/


void process_message(const message *msg, int socket) {
    switch(msg->type) {
        case REGISTER:
            handle_register(msg->client_name, socket);
            break;
        case RESULT:
            handle_result((const arith_resp *) msg->data, msg->client_name);
            break;
        case UNREGISTER:
            handle_unregister(msg->client_name);
            break;
        case PING:
            handle_ping(socket);
            break;
        default:
            fprintf(stderr, "Unexpected message type %#08X at socket %d\n", msg->type, socket);
    }
}


void handle_register(const char *name, int socket) {
    int available_idx = -1;
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(available_idx == -1 && clients[i].socket <= 0) {
            available_idx = i;
        } else if(strcmp(clients[i].name, name) == 0) {
            // name exists
            fprintf(stderr, "Refusing registration of duplicate name '%.*s'\n", MAX_NAME, name);
            send_message(socket, NAME_TAKEN, NULL, 0);
            return;
        }
    }

    strncpy(clients[available_idx].name, name, MAX_NAME);
    clients[available_idx].socket = socket;
    clients[available_idx].responsive = true;
    ++client_count;

    printf("Registered client '%.*s'\n", MAX_NAME, name);
    send_message(socket, REGISTER_ACK, NULL, 0);
}


void handle_result(const arith_resp *resp, const char *client_name) {
    printf("Client '%s' calculated #%d as %d\n", client_name, resp->id, resp->result);
}


void handle_unregister(const char *name) {
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(strcmp(clients[i].name, name) == 0) {
            printf("Client '%.*s' unregistered\n", MAX_NAME, name);

            shutdown(clients[i].socket, SHUT_RDWR);
            close(clients[i].socket);
            clients[i].socket = 0;
            clients[i].name[0] = '\0';
            --client_count;

            break;
        }
    }
}

void handle_ping(int socket) {
    for(int i = 0; i< MAX_CLIENTS; ++i) {
        if(clients[i].socket == socket) {
            clients[i].responsive = true;
            fprintf(stderr, "Client '%s' responded to ping\n", clients[i].name);
            break;
        }
    }
}

/*********************************************************************************
* Input reader
*********************************************************************************/

void *reader(void * arg) {
    (void) arg; // unused
    char *line = NULL;
    size_t n = 0;
    while(true) {
        OK(getline(&line, &n, stdin), "Error reading line from stdin");
        tokens *expr = tokenize(line);
        if(expr->size < 3) {
            fprintf(stderr, "You must provider <operand> <operator> <operand>\n");
            free(expr);
        } else {
            arith_req req = {
                    .id = op_id++,
                    .op = char_to_op(expr->toks[1][0]),
                    .arg1 = atoi(expr->toks[0]),
                    .arg2 = atoi(expr->toks[2])
            };
            free(expr);

            client *c = get_random_client();
            if(c == NULL) {
                printf("No client registered to handle the request\n");
                continue;
            }
            send_message(c->socket, ARITH, &req, sizeof(arith_req));
        };
    }
}


// Splits string on whitespace
tokens *tokenize(char *string) {
    tokens *result = calloc(1, sizeof(tokens));
    size_t wordlen;

    // find beginning of string
    string += strspn(string, WHITESPACE);

    while(string != NULL && *string != '\0' && result->size < MAX_TOKENS) {
        wordlen = strcspn(string, WHITESPACE);
        result->toks[result->size] = calloc(1, wordlen + 1);
        strncpy(result->toks[result->size], string, wordlen);
        result->toks[result->size][wordlen] = '\0';
        ++result->size;

        string += wordlen;
        string += strspn(string, WHITESPACE); // skip whitespace
    }
    return result;
}

arith_op char_to_op(char c) {
    switch(c) {
        case '+':
            return ADD;
        case '-':
            return SUB;
        case '*':
            return MUL;
        case '/':
            return DIV;
        default:
            fprintf(stderr, "Unknown arithmetic operator\n");
            exit(1);
    }
}

client *get_random_client(void) {
    if(client_count < 1) {
        return NULL;
    }
    int i = rand() % MAX_CLIENTS;
    while(clients[i].socket <= 0) {
        i = (i + 1) % MAX_CLIENTS;
    }
    return clients + i;
}

/*********************************************************************************
* Monitor
*********************************************************************************/

void *monitor(void * arg) {
    (void) arg; // unused
    while(true) {
        for(int i = 0; i < MAX_CLIENTS; ++i) {
            if(clients[i].socket <= 0)
                continue;
            if(!clients[i].responsive) {
                fprintf(stderr, "Client '%s' did not respond to ping, removing\n", clients[i].name);
                handle_unregister(clients[i].name);
            } else {
                send_message(clients[i].socket, PING, NULL, 0);
                clients[i].responsive = false;
            }
        }

        sleep(PING_PERIOD);
    }
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

client *find_client_by_name(const char *name) {
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(strcmp(clients[i].name, name) == 0) {
            return &clients[i];
        }
    }
    return NULL;
}

client *find_client_by_socket(const int socket) {
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(clients[i].socket == socket) {
            return &clients[i];
        }
    }
    return NULL;
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
    close(unix_socket);
    OK(close(inet_socket), "Error closing inet socket");
    OK(unlink(unix_socket_path), "Error unlinking local socket");
}