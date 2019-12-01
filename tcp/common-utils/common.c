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

History *empty_history() {
    History *history = (History *) malloc(sizeof(History));

    /* for each part of the msg we will allocate new buffer */
    history->name_body = allocate_char_buffer(EMPTY);
    history->message_body = allocate_char_buffer(EMPTY);

    /* header must be exactly HEADER_SIZE bytes */
    history->message_header = (size_t *) malloc(HEADER_SIZE);
    history->name_header = (size_t *) malloc(HEADER_SIZE);
    history->bytes_left = (size_t *) malloc(HEADER_SIZE);

    *history->bytes_left = HEADER_SIZE;
    return history;
}

Client *empty_client(int *fd) {
    Client *client = (Client *) malloc(sizeof(Client));
    client->state = ST_NAME_HEADER;
    client->id = *fd;
    client->history = empty_history();
    return client;
}

void freeHistory(History *history) {
    free(history->message_header);
    free(history->name_header);
    free(history->message_body);
    free(history->name_body)
    free(history->bytes_left);
    free(history);
}

void free_client(Client *client) {
    freeHistory(client->history);
    free(client);
}

List *list() {
    List *cache = (List *) malloc(sizeof(List));
    *cache = (struct List) {0, NULL};
    return cache;
}

char *allocate_char_buffer(size_t size) {
    return (char *) malloc(sizeof(char) * (size + 1));
}