#include <ntsid.h>
#include <memory.h>
#include <unistd.h>
#include "headers/common.h"
#include "headers/errors.h"
#include "headers/list.h"
#include "headers/io.h"

#define COLON " : "

Reader *new_reader(int exit_code, size_t size) {
    Reader *reader = (Reader *) malloc(sizeof(Reader));
    reader->exit_code = exit_code;
    reader->value = (char *) malloc(sizeof(char) * (size + 1));
    return reader;
}

Reader *empty() {
    return new_reader(FAILURE, 1);
}

Reader *read_shared(Client *client) {
    ssize_t n;
    size_t message_size = 0;

    /* firstly read message size, and then message */
    n = read(client->id, (void *) &message_size, HEADER_SIZE);
    if (n <= 0) {
        return empty();
    } else {
        Reader *reader = new_reader(SUCCESS, message_size);
        n = read(client->id, reader->value, message_size);
        if (n <= 0) {
            free_reader(reader);
            return empty();
        } else return reader;
    }
}

Reader *read_clientname(int fd) {
    Client *client = empty_client(&fd);
    Reader *reader = read_shared(client);
    free_client(client);
    return reader;
}

Reader *read_message(Client *client) {
    return read_shared(client);
}

void send_message(int fd, char *message) {
    size_t info_size = strlen(message) + 1;
    ssize_t n;

    /* firstly write message size as header to client, and then message */
    n = write(fd, &info_size, HEADER_SIZE);
    if (n < 0) raise_error(SOCKET_WRITE_ERROR);
    n = write(fd, message, info_size);
    if (n < 0) raise_error(SOCKET_WRITE_ERROR);
}

void free_reader(Reader *reader) {
    free(reader->value);
    free(reader);
}