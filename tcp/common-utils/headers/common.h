#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include <stdlib.h>
#include <ntsid.h>
#include <pthread.h>
#include "inet_utils.h"
#include "fsm.h"
#include "list.h"

#define MESSAGE_SIZE 1024
#define CLIENT_NAME_SIZE 16
#define HEADER_SIZE sizeof(size_t)
#define MAX_CLIENTS 256

#define EMPTY 1

#define TRUE 1
#define FALSE 0

typedef struct History {
    size_t *name_header;
    char *name_body;
    size_t *message_header;
    char *message_body;
    size_t *bytes_left;
} History;

typedef struct Client {
    int id; // id stores unique socketfd
    State state;
    History *history;
} Client;

List *list();

Client *new_client(int *id, char *name);

Client *empty_client(int *fd);

uint16_t exclude_servport(int argc, char *argv[]);

uint16_t exclude_cliport(int argc, char *argv[]);

void free_client(Client *client);

char *allocate_char_buffer(size_t size);

#endif //SERVER_COMMON_H
