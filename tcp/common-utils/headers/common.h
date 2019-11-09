#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include <stdlib.h>
#include <ntsid.h>
#include <pthread.h>

#define MESSAGE_SIZE 1024
#define CLIENT_NAME_SIZE 16
#define HEADER_SIZE sizeof(size_t)

#define EMPTY 1

typedef struct Client {
    char *name;
    int is_disconnected;
    int id; // id stores unique socketfd
} Client;

typedef struct Env {
    void *cache;
    pthread_mutex_t *blocker;
} Env;

Env *init_env();

Client *new_client(int *id, char *name);

Client *empty_client(int *fd);

uint16_t exclude_servport(int argc, char *argv[]);

uint16_t exclude_cliport(int argc, char *argv[]);

void free_client(Client *client);

char *allocate_char_buffer(size_t size);

#endif //SERVER_COMMON_H
