#ifndef SERVER_READER_H
#define SERVER_READER_H

#include "common.h"

#define SUCCESS 0
#define FAILURE 1

typedef struct Reader {
    int exit_code;
    char *value;
} Reader;

Reader *read_clientname(int fd);

Reader *read_message(Client *client);

void send_message(int fd, char *message);

void free_reader(Reader *reader);

#endif //SERVER_READER_H
