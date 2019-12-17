#include "headers/common.h"
#include "headers/errors.h"
#include "headers/list.h"

#define DECIMAL 10

/*
 * By the returned number of 'atoi' function it is impossible to determine
 * whether the string contains the correct number or not, because in
 * cases when argv = 0 and in error cases it will return 0.
 */
uint16_t exclude(int argc, int argnum, char *argv[], int portnum) {
    if (argc != argnum) raise_error(WRONG_ARGS_NUMBER);
    char *p;
    int port = (int) strtol(argv[portnum], &p, DECIMAL);
    if (port <= 0) raise_error(WRONG_PORT_FORMAT);
    return (uint16_t) port;
}

uint16_t exclude_servport(int argc, char *argv[]) {
    return exclude(argc, 2, argv, 1);
}

uint16_t exclude_cliport(int argc, char *argv[]) {
    return exclude(argc, 3, argv, 2);
}

Client *new_client(int *id, char *name) {
    Client *client = (Client *) malloc(sizeof(Client));
    client->id = *id;
    client->name = name;
    client->is_disconnected = 0;
    return client;
}

Client *empty_client(int *fd) {
    Client *client = (Client *) malloc(sizeof(Client));
    client->id = *fd;
    client->name = allocate_char_buffer(EMPTY);
    client->is_disconnected = 0;
    return client;
}

void free_client(Client *client) {
    free(client->name);
    free(client);
}

Env *init_env() {
    List *cache = (List *) malloc(sizeof(List));
    *cache = (struct List) {0, NULL};

    pthread_mutex_t blocker;
    if (pthread_mutex_init(&blocker, NULL) != 0) {
        raise_error(MUTEX_INIT_ERROR);
    }

    Env *init_structure = (Env *) malloc(sizeof(Env));
    *init_structure = (struct Env) {cache, &blocker};
    return init_structure;
}

char *allocate_char_buffer(size_t size) {
    return (char *) malloc(sizeof(char) * (size + 1));
}