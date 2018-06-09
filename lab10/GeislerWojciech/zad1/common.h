// Wojciech Geisler
// 2018-06

#ifndef LAB10_COMMON_H
#define LAB10_COMMON_H

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <netinet/in.h>

#define OK(_EXPR, _ERR_MSG) if((_EXPR) < 0) { fprintf(stderr, "%s: %d %s\n", _ERR_MSG, errno, strerror(errno)); exit(1); }

#define UNIX_PATH_MAX    108
#define MAX_NAME 20
#define MAX_CLIENTS 20
#define MAX_LEN 64

typedef enum {
    REGISTER = 1,
    REGISTER_ACK,
    NAME_TAKEN,
    MIRROR,
    ARITH,
    RESULT,
    UNREGISTER
} msg_type;

typedef enum arith_op {
    ADD,
    MUL,
    SUB,
    DIV
} arith_op;

typedef struct {
    msg_type type;
    char client_name[MAX_NAME];
    size_t len;
    char data[MAX_LEN];
} message; // parent type

typedef struct {
    arith_op op;
    int arg1;
    int arg2;
} arith_req;


#endif //LAB10_COMMON_H
