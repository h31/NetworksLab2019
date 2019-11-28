#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "../common-utils/headers/io.h"
#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"
#include "../common-utils/headers/list.h"
#include "../common-utils/headers/async_io.h"

#define MAX_QUEUED_CLIENTS 7
#define TIMEOUT -1

#define COLON " : "

// https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_71/rzab6/poll.html

List *cache = NULL;

void send_message_to_clients(Client *sender) {
    char *message = allocate_char_buffer(
            strlen(sender->history->name) + strlen(COLON) + strlen(sender->history->message));
    /* insert client name, colon and message */
    strcpy(message, sender->history->name);
    strcat(message, COLON);
    strcat(message, sender->history->message);

    foreach(&send_message, message, cache);

    // TODO ??? SHOULD WE FREE SENDER HISTORY???
}

void which_client_scenario(Client *client) {
    switch (client->state) {
        case ST_NAME_HEADER:
            read_header(client);
            break;
        case ST_MESSAGE_HEADER:
            read_header(client);
            break;
        case ST_NAME:
            read_message(client);
            break;
        case ST_MESSAGE:
            read_message(client);
            send_message_to_clients(client);
            break;
    }
}

void accept_new_cient(Poll_vector *vector) {
    int new_socketfd = 0;
    new_socketfd = accept(vector->descriptors[0].fd, NULL, NULL);
    if (new_socketfd < 0 && errno != EWOULDBLOCK) raise_error(INTERNAL_ERROR);
    else { // Add the new incoming connection
        vector->descriptors[vector->length].fd = new_socketfd;
        vector->descriptors[vector->length].events = POLL_IN;
        vector->length++;

        push(cache, empty_client(&new_socketfd));
    }
}


void start_event_loop(Poll_vector *vector) {
    int rc = poll(vector->descriptors, vector->length, TIMEOUT);
    if (rc < 0) raise_error(POLL_ERROR);

    for (int i = 0; i < (int) vector->length; ++i) {
        poll_descriptor current = vector->descriptors[i];
        if (current.revents == 0) continue; // TODO ???
        if (current.revents != POLL_IN) raise_error(POLL_ERROR);
        /* listening socket */
        if (current.fd == vector->descriptors[0].fd) accept_new_cient(vector);
        else {
            /* readable connection */
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