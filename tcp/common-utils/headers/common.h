#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include <stdlib.h>
#include <ntsid.h>
#include <pthread.h>

typedef struct Client {
    char *name;
    int id; // id stores unique socketfd
} Client;

typedef struct Env {
    void *cache;
    pthread_mutex_t *blocker;
} Env;

Env *init_env();

Client *allocate_client(int *id, char *name);

uint16_t exclude_servport(int argc, char *argv[]);

uint16_t exclude_cliport(int argc, char *argv[]);

#endif //SERVER_COMMON_H
