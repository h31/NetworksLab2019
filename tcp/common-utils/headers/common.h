#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include <stdlib.h>

uint16_t exclude_servport(int argc, char *argv[]);

uint16_t exclude_cliport(int argc, char *argv[]);

typedef struct Client {
    char *name;
    long id;
} Client;

#endif //SERVER_COMMON_H
