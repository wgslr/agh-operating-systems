// Wojciech Geisler
// 2018-04

#ifndef LAB6_CONSTANTS_H
#define LAB6_CONSTANTS_H

#include <limits.h>

#define CONTENT_SZ 128
#define MSG_SZ (sizeof(msgbuf) - sizeof(long))
#define MAX_CLIENTS 128
#define FTOK_PROJ_ID 123
#define SERVER_ID INT_MAX

#define OK(_EXPR, _ERR_MSG) if((_EXPR) < 0) { fprintf(stderr, "%s: %d %s\n", _ERR_MSG, errno, strerror(errno)); exit(1); }

typedef enum {
    REGISTER = 1, // mtype must be > 0
    REGISTER_RESP,
    MIRROR,
    MIRROR_RESP,
    CALC,
    CALC_RESP,
    TIME,
    TIME_RESP,
    STOP,
    END,
} MSG_TYPE;


typedef struct msgbuf {
    long mtype;
    int sender_id;
    int sender_pid;
    char content[CONTENT_SZ];
} msgbuf;

typedef enum arith_op {
    ADD,
    MUL,
    SUB,
    DIV
} arith_op;

typedef struct arith_req {
    arith_op op;
    int arg1;
    int arg2;
} arith_req;


#endif //LAB6_CONSTANTS_H
