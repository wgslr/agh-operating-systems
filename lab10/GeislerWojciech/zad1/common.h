// Wojciech Geisler
// 2018-06

#ifndef LAB10_COMMON_H
#define LAB10_COMMON_H

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <netinet/in.h>

#define OK(_EXPR, _ERR_MSG) if((_EXPR) < 0) { fprintf(stderr, "%s: %d %s\n", _ERR_MSG, errno, strerror(errno)); exit(1); }

#define UNIX_PATH_MAX    108
#define MAX_NAME 23
#define MAX_CLIENTS 20
#define MAX_LEN 64
#define MAX_TOKENS 5
#define PING_PERIOD 15

typedef enum {
    REGISTER = 0x11,
    REGISTER_ACK = 0x22,
    NAME_TAKEN = 0x33,
    ARITH = 0x44,
    RESULT = 0x55,
    UNREGISTER = 0x66,
    PING = 0x77,
} msg_type;

typedef enum arith_op {
    ADD,
    MUL,
    SUB,
    DIV
} arith_op;

typedef struct {
    uint16_t type;
    uint32_t len;
    char client_name[MAX_NAME + 1];
    uint32_t data[0]; // usign uint32_t for alignment portability
} message; // parent type

typedef struct {
    int32_t id;
    int32_t op; //arith_op
    int32_t arg1;
    int32_t arg2;
} arith_req;

typedef struct {
    int32_t id;
    int32_t result;
} arith_resp;

void dumpmem(void * pointer, size_t len) {
    uint8_t *p = pointer;
    for(size_t i = 0; i < len; ++i) {
        printf("%02hX", *(p + i));
        if(i % 4 == 3)
            printf(" ");
    }
    printf("\n");
}


#endif //LAB10_COMMON_H
