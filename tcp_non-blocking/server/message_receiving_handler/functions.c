#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include "../constants.h"
#include "../list_of_connections/list_of_connections.h"
#include "../list_of_users/list_of_users.h"


int read_data_(int fd, void* buffer) {
    bzero(buffer, MAX_RECEIVED_MESSAGE_SIZE);

    int number_read = read(fd, buffer, MAX_RECEIVED_MESSAGE_SIZE);

    if ( number_read < 0) {
        fprintf(stdout, "ERROR: error while reading from socket '%d'", fd);
    }

    return number_read;
}


void remove_all_user_info(int user_sockfd) {
    list_of_users_remove(user_sockfd);
    list_of_connections_remove(user_sockfd);
}


void read_messages_() {
    for (int user_index = 0; user_index < MAX_NUMBER_OF_CONNECTED_USERS; ++user_index) {
        int revents = list_of_connections_get_item(user_index) -> revents;

        if (revents == POLLIN) {
            void* data = malloc(MAX_RECEIVED_MESSAGE_SIZE);
            if (read_data_(list_of_connections_get_item(user_index)->fd, data) != 0) {
                list_of_users_add_data(list_of_connections_get_item(user_index)->fd, data);
            } else {
                remove_all_user_info(list_of_connections_get_item(user_index) -> fd);
            }
            free(data);
        } else if (revents == POLLHUP || revents == POLLNVAL) {
            remove_all_user_info(list_of_connections_get_item(user_index) -> fd);
        }
    }
}


void message_receiving_handler_read_messages() {
    if (list_of_connections_number_of_sockets_with_data() > 0) {
        read_messages_();
    }
}