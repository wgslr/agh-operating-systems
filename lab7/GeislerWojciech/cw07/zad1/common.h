//
// Created by wojciech on 5/8/18.
//

#ifndef LAB7_COMMON_H
#define LAB7_COMMON_H

#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <memory.h>
#include <errno.h>

#define FTOK_PROJ_ID 123
#define SEMS 3
#define OK(_EXPR, _ERR_MSG) if((_EXPR) < 0) { fprintf(stderr, "%s: %d %s\n", _ERR_MSG, errno, strerror(errno)); exit(1); }

#define EMPTY_SEATS_SEM
#define IS_SLEEPING_SEM

void logmsg(const char *msg);
key_t get_ipc_key(void);

#endif //LAB7_COMMON_H
