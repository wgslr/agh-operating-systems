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
    sa_family_t family;
    struct sockaddr addr;
    socklen_t addrlen;
} conn;

typedef struct {
    char name[MAX_NAME + 1];
    conn addr;
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

client clients[MAX_CLIENTS] = {{{0}, {0}, 0}};

/*********************************************************************************
* Predeclarations
*********************************************************************************/

void process_message(const message *msg, const conn *sender);

int open_network_socket(const short port);

int open_local_socket(const char *path);

void handle_register(const char *name, const conn *sender);

void cleanup(void);

message *read_message(int socket, conn *sender);

void accept_connection(int socket, int epoll_fd);

// Spawns a thread with default attributes
pthread_t spawn(void *(*func)(void *), void *args);

void *listener(void *arg);

int char_to_op(char c, arith_op *op);

client *get_random_client(void);

void *reader(void *);

// Splits string on whitespace
tokens *tokenize(char *string);

void handle_result(const arith_resp *resp, const char *client_name);

void handle_unregister(const char *name);

void handle_ping(const char *name);

void *monitor(void *);

void send_message(const conn *addr, msg_type type, void *data, uint32_t len);

client *find_client_by_name(const char *name) ;

void sigint(int signum) ;

/*********************************************************************************
* Main code
*********************************************************************************/


int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Incorrect number of arguments (expected 2)\n");
        exit(1);
    }

    srand((unsigned int) time(NULL));
    atexit(&cleanup);
    signal(SIGINT, &sigint);

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

    fprintf(stdout, "Server listening at 0.0.0.0:%d and '%s'\n", args->port, args->path);

    while(true) {
        OK(epoll_wait(epoll_fd, &event, 1, -1), "Error waiting for message");

        int event_socket = event.data.fd;
        conn sender = {
                .family = event_socket == unix_socket ? AF_UNIX : AF_INET
        };
        message *msg = read_message(event_socket, &sender);
        if(msg != NULL) {
            process_message(msg, &sender);
        }
        free(msg);
    }
}

int open_network_socket(const short port) {
    const struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port =  htons(port),
            .sin_addr.s_addr = INADDR_ANY
    };

    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    OK(fd, "Error opening inet socket");

    // prevent error after restrt
    int val = 1;
    OK(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)), "Error setting socket opt");

    OK(bind(fd, (const struct sockaddr *) &addr, sizeof(addr)), "Error binding inet socket");
    return fd;
}

int open_local_socket(const char *path) {
    struct sockaddr_un addr = {
            .sun_family = AF_UNIX,
    };
    strncpy(addr.sun_path, path, UNIX_PATH_MAX);

    const int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    OK(fd, "Error opening unix socket");
    OK(bind(fd, (struct sockaddr *) &addr, sizeof(addr)), "Error binding unix socket");
    return fd;
}

message *read_message(int socket, conn *sender) {
    ssize_t bytes;
    message *buff = calloc(1, sizeof(message));
    sender->addrlen = sizeof(struct sockaddr);

    bytes = recvfrom(socket, buff, sizeof(message), 0, &sender->addr, &sender->addrlen);
    OK(bytes, "Error receiving message")

    if(bytes == 0) {
        fprintf(stderr, "Received empty message\n");
        free(buff);
        buff = NULL;
    }

    return buff;
}

void send_message(const conn *addr, msg_type type, void *data, uint32_t len) {
    int socket = addr->family == AF_UNIX ? unix_socket : inet_socket;

    message *msg = calloc(1, sizeof(message));
    msg->type = type;
    msg->len = len;
    assert(len <= MAX_LEN);
    if(len > 0) {
        memcpy(msg->data, data, len);
    }

    OK(sendto(socket, msg, sizeof(message), 0, &addr->addr, addr->addrlen), "Error sending message");
    free(msg);
}


/*********************************************************************************
* Message handler
*********************************************************************************/


void process_message(const message *msg, const conn *sender) {
    switch(msg->type) {
        case REGISTER:
            handle_register(msg->client_name, sender);
            break;
        case RESULT:
            handle_result((const arith_resp *) msg->data, msg->client_name);
            break;
        case UNREGISTER:
            handle_unregister(msg->client_name);
            break;
        case PING:
            handle_ping(msg->client_name);
            break;
        default:
            fprintf(stderr, "Unexpected message type %#08X\n", msg->type);
    }
}


void handle_register(const char *name, const conn *sender) {
    if(client_count >= MAX_CLIENTS) {
        fprintf(stderr, "Clients registry full, refusing registration\n");
        return;
    }

    int available_idx = -1;
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(available_idx == -1 && clients[i].name[0] == '\0') {
            available_idx = i;
        } else if(strcmp(clients[i].name, name) == 0) {
            // name exists
            fprintf(stderr, "Refusing registration of duplicate name '%.*s'\n", MAX_NAME, name);
            send_message(sender, NAME_TAKEN, NULL, 0);
            return;
        }
    }

    ++client_count;
    strncpy(clients[available_idx].name, name, MAX_NAME);
    memcpy(&clients[available_idx].addr, sender, sizeof(conn));
    clients[available_idx].responsive = true;

    printf("Registered client '%.*s'. %d clients online\n", MAX_NAME, name, client_count);
    send_message(sender, REGISTER_ACK, NULL, 0);
}


void handle_result(const arith_resp *resp, const char *client_name) {
    printf("Client '%s' calculated #%d as %d\n", client_name, resp->id, resp->result);
}


void handle_unregister(const char *name) {
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(strcmp(clients[i].name, name) == 0) {
            printf("Client '%.*s' unregistered\n", MAX_NAME, name);
            clients[i].name[0] = '\0';
            --client_count;
            break;
        }
    }
}

void handle_ping(const char *name) {
    client *c = find_client_by_name(name);
    c->responsive = true;
    fprintf(stderr, "CLient '%s' responded to ping\n", c->name);
}

/*********************************************************************************
* Input reader
*********************************************************************************/

void *reader(void *arg) {
    (void) arg; // unused
    char *line = NULL;
    size_t n = 0;
    while(true) {
        OK(getline(&line, &n, stdin), "Error reading line from stdin");
        tokens *expr = tokenize(line);
        if(expr->size < 3) {
            printf("You must provider <operand> <operator> <operand>\n");
            free(expr);
        } else {
            arith_req req = {
                    .id = op_id++,
                    .arg1 = atoi(expr->toks[0]),
                    .arg2 = atoi(expr->toks[2])
            };

            arith_op op;
            if(char_to_op(expr->toks[1][0], &op) != 0) {
                fprintf(stdout, "Unknown arithmetic operator\n");
                free(expr);
                continue;
            } else {
                req.op = op;
                free(expr);
            }



            client *c = get_random_client();
            if(c == NULL) {
                printf("No client registered to handle the request\n");
            } else {
                printf("Sending request #%d to '%.*s'\n", req.id, MAX_NAME, c->name);
                send_message(&c->addr, ARITH, &req, sizeof(arith_req));
            }
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

int char_to_op(char c, arith_op *op) {
    switch(c) {
        case '+':
            *op = ADD;
            break;
        case '-':
            *op = SUB;
            break;
        case '*':
            *op = MUL;
            break;
        case '/':
            *op = DIV;
            break;
        default:
            return -1;
    }
    return 0;
}

client *get_random_client(void) {
    if(client_count < 1) {
        return NULL;
    }
    int i = rand() % MAX_CLIENTS;
    while(clients[i].name[0] == '\0') {
        i = (i + 1) % MAX_CLIENTS;
    }
    return clients + i;
}

/*********************************************************************************
* Monitor
*********************************************************************************/

void *monitor(void *arg) {
    (void) arg; // unused
    while(true) {
        for(int i = 0; i < MAX_CLIENTS; ++i) {
            if(clients[i].name[0] == '\0')
                continue;
            if(!clients[i].responsive) {
                fprintf(stderr, "Client '%s' did not respond to ping, removing\n", clients[i].name);
                handle_unregister(clients[i].name);
            } else {
                send_message(&clients[i].addr, PING, NULL, 0);
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

void sigint(int signum) {
    (void) signum;
    exit(0);
}

void cleanup(void) {
    CHECK(shutdown(inet_socket, SHUT_RDWR), "Shutdown error of local socket");
    CHECK(shutdown(unix_socket, SHUT_RDWR), "Shutdown error of unix socket");
    CHECK(close(unix_socket), "Error closing local socket");
    CHECK(close(inet_socket), "Error closing inet socket");
    CHECK(unlink(unix_socket_path), "Error unlinking local socket");

    fprintf(stderr, "Cleanup finished, exiting\n");
}