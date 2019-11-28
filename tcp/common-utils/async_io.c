#include <errno.h>
#include "headers/common.h"
#include "unistd.h"
#include "headers/errors.h"

/*
 * Bad functions used to read info from socket
 * and update client's internal state
 */


void which_header_state(Client *client, size_t header_size) {
    switch (client->state) {
        case ST_NAME_HEADER:
            client->history->name_size = client->history->name_size + header_size;
            break;
        case ST_MESSAGE_HEADER:
            client->history->message_size = client->history->message_size + header_size;
        default:
            raise_error(SHOULD_NOT_BE);
    }
}

void which_message_state(Client *client, char *message) {
    switch (client->state) {
        case ST_NAME:
            strcpy(client->history->name, message);
            break;
        case ST_MESSAGE:
            strcpy(client->history->message, message);
            break;
        default:
            raise_error(SHOULD_NOT_BE);
    }
}

void read_header(Client *client) {
    ssize_t n;
    size_t header_size = 0;

    n = read(client->id, (void *) &header_size, HEADER_SIZE);
    if (n < 0 && errno != EWOULDBLOCK) {
        free_client(client);
        raise_error(SOCKET_READ_ERROR);
    }

    which_header_state(client, header_size); // TODO check if int = 0 after malloc
    if (errno != EWOULDBLOCK) client->enough_data = FALSE;
    else {
        client->enough_data = TRUE;
        client->state = next(client->state);
    }
}

void read_message(Client *client) {
    ssize_t n;
    // TODO IF IT WORKS???
    n = read(client->id, client->history->message, client->history->message_size);
    if (n < 0) {
        free_client(client);
    }
}