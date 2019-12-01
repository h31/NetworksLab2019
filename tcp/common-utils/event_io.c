#include "headers/common.h"
#include "unistd.h"

void read_header(size_t *header, char *message, Client *client) {
    ssize_t n;
    size_t received_size = 0;

    n = read(client->id, (void *) received_size, *client->history->bytes_left);
    // TODO CHECK FOR ERRORS

    *client->history->bytes_left -= n;
    *header += received_size;
    if (*client->history->bytes_left == EMPTY) {
        client->state = next(client->state);
        *message = *allocate_char_buffer(*header);
        *client->history->bytes_left = *header;
    }
}

void read_message_header(Client *client) {
    read_header(client->history->message_header, client->history->message_body, client);
}

void read_name_header(Client *client) {
    read_header(client->history->name_header, client->history->name_body, client);
}

void read_message(Client *client) {
    ssize_t n;
    // TODO IF IT WORKS???
    n = read(client->id, client->history->message, client->history->message_size);
    if (n < 0) {
        free_client(client);
    }
}