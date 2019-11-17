#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "../common-utils/headers/io.h"
#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"
#include "../common-utils/headers/list.h"

#define MAX_QUEUED_CLIENTS 7
#define TIMEOUT -1

#define JOIN " joined to the chat!"
#define COLON " : "

// https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_71/rzab6/poll.htm

List *cache = NULL;

void start_communication(Client *client) {
    while (!client->is_disconnected) {
        Reader *reader = read_message(client);
        if (reader->exit_code == SUCCESS) {
            char *message = allocate_char_buffer(strlen(client->name) + strlen(COLON) + strlen(reader->value));
            /* insert client name, colon and message */
            strcpy(message, client->name);
            strcat(message, COLON);
            strcat(message, reader->value);

            foreach(&send_message, message, cache);
            free(message);
        }
        free_reader(reader);
    }
    printf("<logger>: client left the chat, name: %s\n", client->name);
    delete(cache, client);
}

void start_client_scenario(void *args) {
    int *client_id = (int *) args;

    Reader *reader = read_clientname(*client_id);
    if (reader->exit_code == FAILURE) {
        free_reader(reader);
        raise_error(SOCKET_READ_ERROR);
    }

    /* copying client_name is necessary for reader deallocating */
    size_t len = strlen(reader->value);
    char *client_name = allocate_char_buffer(len);
    strcpy(client_name, reader->value);
    Client *client = new_client(client_id, client_name);

    free_reader(reader);

    /* apply client to cache */
    push(cache, client);

    char *welcome_msg = allocate_char_buffer(strlen(client->name) + strlen(JOIN));
    /* insert client name and join literal */
    strcpy(welcome_msg, client_name);
    strcat(welcome_msg, JOIN);

    printf("<logger>: %s\n", welcome_msg);

    foreach(&send_message, welcome_msg, cache);
    free(welcome_msg);
    start_communication(client);
}

void which_client_scenario(Client *client) {
    switch (client->state){
        case FHEADER_SIZE:

    }
}

void accept_new_cients(Poll_vector *vector) {
    int new_socketfd = 0;
    while (new_socketfd != -1) {
        new_socketfd = accept(vector->descriptors[0].fd, NULL, NULL);
        if (new_socketfd < 0 && errno != EWOULDBLOCK) raise_error(INTERNAL_ERROR);
        else { // Add the new incoming connection
            vector->descriptors[vector->length].fd = new_socketfd;
            vector->descriptors[vector->length].events = POLL_IN;
            vector->length++;

            push(cache, empty_client(&new_socketfd));
        }
    }
}


void start_event_loop(Poll_vector *vector) {
    int rc = poll(vector->descriptors, vector->length, TIMEOUT);
    if (rc < 0) raise_error(POLL_ERROR);

    for (int i = 0; i < (int) vector->length; ++i) {
        poll_descriptor current = vector->descriptors[i];
        if (current.revents == 0) continue;
        if (current.revents != POLL_IN) raise_error(POLL_ERROR);
        if (current.fd == vector->descriptors[0].fd) accept_new_cients(vector);
        else {
            Client *current_client = get_by_id(cache, current.fd);
            which_client_scenario(current_client);
        }
    }
}

void start_server(const uint16_t *port) {
    int sockfd = create_tcpsocket();
    /* allow socket descriptor to be reuseable and set socket to be nonblocking */
    async_socket(sockfd);

    socket_descriptor serv_addr;
    set_servsockdesc(&serv_addr, *port);

    if (bind(sockfd, (address *) &serv_addr, sizeof(serv_addr)) < 0) {
        raise_error(BINDING_ERROR);
    }

    if (listen(sockfd, MAX_QUEUED_CLIENTS) < 0) {
        raise_error(SOCKET_STATE_ERROR);
    }

    /* initialize server environment */
    Env *env = init_env(sockfd);
    cache = env->cache;
    Poll_vector *vector = env->vector;

    for (;;) { start_event_loop(vector); }
}