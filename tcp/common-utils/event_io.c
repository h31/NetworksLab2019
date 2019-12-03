#include "headers/common.h"
#include "unistd.h"

void read_header(size_t *header, char *message, Client *client) {
    ssize_t n;
    size_t received_size = 0;

    n = read(client->id, (char *) &received_size, *client->history->bytes_left);
    // TODO CHECK FOR ERRORS

    *client->history->bytes_left -= n;
    *header += received_size;
    if (*client->history->bytes_left == 0) {
        client->state = next(client->state);
        *message = *allocate_char_buffer(*header);
        // for future message value
        *client->history->bytes_left = *header;
    }
}

void read_message_header(Client *client) {
    read_header(client->history->message_header, client->history->message_body, client);
}

void read_name_header(Client *client) {
    read_header(client->history->name_header, client->history->name_body, client);
}

char *read_message(Client *client) {
    ssize_t n;
    char *received_message = allocate_char_buffer(*client->history->bytes_left);

    n = read(client->id, received_message, *client->history->bytes_left);
    // TODO CHECK FOR ERRORS

    *client->history->bytes_left -= n;

    if (*client->history->bytes_left == 0) {
        client->state = next(client->state);
        *client->history->bytes_left = HEADER_SIZE;
    }
    return received_message;
}

void read_message_body(Client *client) {
    char *received_message = read_message(client);
    char *message = client->history->message_body;

    char *new_message = allocate_char_buffer(strlen(message) + strlen(received_message));
    strcpy(new_message, message);
    strcat(new_message, received_message);

    free(client->history->message_body);

    client->history->message_body = new_message;
}

void read_name_body(Client *client) {
    char *received_message = read_message(client);
    char *message = client->history->name_body;

    char *new_message = allocate_char_buffer(strlen(message) + strlen(received_message));
    strcpy(new_message, message);
    strcat(new_message, received_message);

    free(client->history->name_body);

    client->history->name_body = new_message;
}