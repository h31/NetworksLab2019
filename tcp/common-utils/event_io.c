#include "headers/common.h"
#include "headers/event_io.h"
#include "unistd.h"

Helper *allocate_helper(char *message, bool exit_code) {
    Helper *helper = (Helper *) malloc(sizeof(Helper));
    helper->message = message;
    helper->exit_code = exit_code;
    return helper;
}

void free_helper(Helper *helper) {
    free(helper->message);
    free(helper);
}


Helper *read_header(size_t *header, char *message, Client *client) {
    ssize_t n;
    size_t received_size = 0;

    n = read(client->id, (char *) &received_size, *client->history->bytes_left);
    Helper *helper;
    if (n == 0) {
        helper = allocate_helper(allocate_char_buffer(EMPTY), false);
    } else {
        *client->history->bytes_left -= n;
        *header += received_size;
        if (*client->history->bytes_left == 0) {
            client->state = next(client->state);
            *message = *allocate_char_buffer(*header);
            // for future message value
            *client->history->bytes_left = *header;
        }
        helper = allocate_helper(allocate_char_buffer(EMPTY), true);
    }
    return helper;
}

bool read_message_header(Client *client) {
    Helper *helper = read_header(client->history->message_header, client->history->message_body, client);
    bool res = helper->exit_code;
    free_helper(helper);
    return res;
}

bool read_name_header(Client *client) {
    Helper *helper = read_header(client->history->name_header, client->history->name_body, client);
    bool res = helper->exit_code;
    free_helper(helper);
    return res;
}

Helper *read_message(Client *client) {
    ssize_t n;
    char *received_message = allocate_char_buffer(*client->history->bytes_left);

    n = read(client->id, received_message, *client->history->bytes_left);
    Helper *helper;
    if (n == 0) {
        helper = allocate_helper(received_message, false);
    } else {
        *client->history->bytes_left -= n;
        if (*client->history->bytes_left == 0) {
            client->state = next(client->state);
            *client->history->bytes_left = HEADER_SIZE;
        }
        helper = allocate_helper(received_message, true);
    }
    return helper;
}

bool read_message_body(Client *client) {
    Helper *helper = read_message(client);
    if (helper->exit_code) {
        char *received_message = helper->message;
        if (*client->history->bytes_left == HEADER_SIZE) {
            *client->history->message_header = 0;
        }
        char *message = client->history->message_body;

        char *new_message = allocate_char_buffer(strlen(message) + strlen(received_message));
        strcpy(new_message, message);
        strcat(new_message, received_message);

        free(client->history->message_body);

        client->history->message_body = new_message;
        free_helper(helper);
        return true;
    } else {
        free(helper);
        return false;
    }
}

bool read_name_body(Client *client) {
    Helper *helper = read_message(client);
    if (helper->exit_code) {
        char *received_message = helper->message;
        if (*client->history->bytes_left == HEADER_SIZE) {
            *client->history->name_header = 0;
        }
        char *message = client->history->name_body;

        char *new_message = allocate_char_buffer(strlen(message) + strlen(received_message));
        strcpy(new_message, message);
        strcat(new_message, received_message);

        free(client->history->name_body);

        client->history->name_body = new_message;
        free(helper);
        return true;
    } else {
        free(helper);
        return false;
    }
}