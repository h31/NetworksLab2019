//
// Created by Malik Hiraev on 10/11/2019.
//

#include "clients_handler.h"

static Client *first_client = NULL;

static Client *last_client = NULL;

static unsigned num_of_clients = 0;

unsigned get_num_of_clients() {
    return num_of_clients;
}

Client *get_first_client() {
    return first_client;
}

void remove_client(Client *dead_client) {
    if (first_client == dead_client) {
        first_client = first_client->next_client;
    }
    if (dead_client->prev_client != NULL) {
        dead_client->prev_client->next_client = dead_client->next_client;
    }
    if (dead_client->next_client != NULL) {
        dead_client->next_client->prev_client = dead_client->prev_client;
    }
    if (dead_client != last_client) {
        last_client->fd_index = dead_client->fd_index;
    } else {
        last_client = last_client->prev_client;
    }
    if (dead_client->message != NULL) {
        if (dead_client->message->msg != NULL) free(dead_client->message->msg);
        free(dead_client->message);
    }
    if (dead_client->name != NULL) free(dead_client->name);
    free(dead_client);
    num_of_clients--;
}

void accept_client(Client *new_client) {
    num_of_clients++;
    if (first_client == NULL) {
        first_client = new_client;
    } else {
        last_client->next_client = new_client;
        new_client->prev_client = last_client;
    }
    last_client = new_client;
}
