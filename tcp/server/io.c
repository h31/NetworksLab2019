#include <ntsid.h>
#include <memory.h>
#include <unistd.h>
#include "../common-utils/headers/common.h"
#include "../common-utils/headers/errors.h"
#include "../common-utils/headers/list.h"
#include "io.h"

int read_shared(Client *client, char *buffer) {
    ssize_t n;
    size_t message_size = 0;
    memset(buffer, 0, sizeof(char));

    /* firstly read message size, and then info */
    n = read(client->id, (void *) &message_size, HEADER_SIZE);
    if (n <= 0) {
        client->is_disconnected = 1;
        return FAILURE;
    } else {
        n = read(client->id, buffer, message_size);
        if (n <= 0) {
            client->is_disconnected = 1;
            return FAILURE;
        }
    }
    return SUCCESS;
}

int read_clientname(int fd, char *name_bf) {
    Client *client = new_client(&fd, EMPTY);
    int exit_code = read_shared(client, name_bf);
    free(client);
    return exit_code;
}

int read_message(Client *client, char *info_bf) {
    return read_shared(client, info_bf);
}

void send_message(Client *client, char *message) {
    size_t info_size = strlen(message);
    ssize_t n;

    /* firstly write info_size as header to client, and then info */
    n = write(client->id, &info_size, HEADER_SIZE);
    if (n < 0) raise_error(SOCKET_WRITE_ERROR);
    n = write(client->id, message, info_size);
    if (n < 0) raise_error(SOCKET_WRITE_ERROR);
}