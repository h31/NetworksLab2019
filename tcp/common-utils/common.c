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

Client *empty_client(int *fd) {
    Client *client = (Client *) malloc(sizeof(Client));
    client->state = ST_NAME_HEADER;
    client->id = *fd;

    History *history = (History *) malloc(sizeof(History));
    history->name = allocate_char_buffer(CLIENT_NAME_SIZE);
    history->message = allocate_char_buffer(MESSAGE_SIZE);
    return client;
}

void freeHistory(History *history) {
    free(history->message);
    free(history->name);
    free(history);
}

void free_client(Client *client) {
    freeHistory(client->history);
    free(client);
}

Env *init_env(int sockfd) {
    List *cache = (List *) malloc(sizeof(List));
    *cache = (struct List) {0, NULL};

    /* set up the initial listening socket */
    poll_descriptor *descriptors = (poll_descriptor *) malloc(MAX_CLIENTS * sizeof(poll_descriptor));
    descriptors[0].fd = sockfd;
    descriptors[0].events = POLLIN;

    Poll_vector *vector = (Poll_vector *) malloc(sizeof(Poll_vector));
    vector->length = 1;
    vector->descriptors = descriptors;

    Env *init_structure = (Env *) malloc(sizeof(Env));
    *init_structure = (struct Env) {cache, vector};
    return init_structure;
}

char *allocate_char_buffer(size_t size) {
    return (char *) malloc(sizeof(char) * (size + 1));
}