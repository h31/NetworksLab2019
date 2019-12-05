#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "../common-utils/headers/io.h"
#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"
#include "../common-utils/headers/list.h"
#include "../common-utils/headers/poll_ops.h"
#include "../common-utils/headers/event_io.h"

#define MAX_QUEUED_CLIENTS 7
#define TIMEOUT -1

#define COLON " : "

// https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_71/rzab6/poll.html

List *cache = NULL;

List *list();

void send_message_to_clients(Client *sender) {
    char *message = allocate_char_buffer(
            strlen(sender->history->name_body) + strlen(COLON) + strlen(sender->history->message_body));
    /* insert client name, colon and message */
    strcpy(message, sender->history->name_body);
    strcat(message, COLON);
    strcat(message, sender->history->message_body);

    foreach(&send_message, message, cache);
    free(message);

    free(sender->history->message_body);
    sender->history->message_body = allocate_char_buffer(EMPTY);
}

bool which_client_scenario(Client *client) {
    switch (client->state) {
        case ST_NAME_HEADER:
            return read_name_header(client);
        case ST_MESSAGE_HEADER:
            return read_message_header(client);
        case ST_NAME:
            return read_name_body(client);
        case ST_MESSAGE:
            if (read_message_body(client)) {
                send_message_to_clients(client);
                return true;
            } else return false;
    }
    return false;
}

void accept_new_cient() {
    int new_socketfd = 0;
    new_socketfd = accept_();
    if (new_socketfd < 0) {
        close_();
        raise_error(INTERNAL_ERROR);
    } else { // Add the new incoming connection
        add_to_descriptors(new_socketfd);
        push(cache, empty_client(&new_socketfd));
    }
}

void start_event_loop() {
    int rc = poll_();
    if (rc < 0) {
        close_();
        raise_error(POLL_ERROR);
    }

    poll_descriptor acceptor = get_acceptor();
    if (acceptor.revents & POLL_IN) accept_new_cient();
    if (acceptor.revents < 0) {
        close_();
        raise_error(POLL_ERROR);
    }

    for (int i = 1; i < get_size(); ++i) {
        poll_descriptor current = getn_from_descriptor(i);
        if (current.revents & POLL_IN) {
            /* readable connection */
            Client *current_client = get_by_id(cache, current.fd);
            bool res = which_client_scenario(current_client);
            if (!res) {
                close_n(current.fd);
                delete(cache, get_by_id(cache, current.fd));
            }
            zero_revents(i);
        } else if (current.revents != 0) {
            /* client is disconnected */
            close_n(current.fd);
            delete(cache, get_by_id(cache, current.fd));
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
    cache = list();
    add_acceptor_to_descriptors(sockfd);

    for (;;) { start_event_loop(); }
}