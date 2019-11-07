#ifndef SERVER_READER_H
#define SERVER_READER_H

#include "common.h"

#define SUCCESS 1
#define FAILURE 0

typedef struct Reader {
    int exit_code;
    char *value;
} Reader;

Reader *read_clientname(int fd);

void send_message(int fd, char *message);

Reader *read_message(Client *client);

int read_header(Client *client, size_t *readed_size);

void free_reader(Reader *reader);

#endif //SERVER_READER_H
