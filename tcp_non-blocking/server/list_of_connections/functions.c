#include <poll.h>
#include "../constants.h"

struct pollfd list_of_connections[MAX_NUMBER_OF_CONNECTED_USERS];


static int get_user_index_(int sockfd) {
    int i = 0;
    while (i < MAX_NUMBER_OF_CONNECTED_USERS && list_of_connections[i].fd != sockfd) {
        i++;
    }
    return (i < MAX_NUMBER_OF_CONNECTED_USERS) ? i : -1;
}


void list_of_connections_init(void) {
    for (int i = 0; i < MAX_NUMBER_OF_CONNECTED_USERS; ++i) {
        list_of_connections[i].fd = -1;
        list_of_connections[i].events = POLLIN;
    }
}


struct pollfd* list_of_connections_get_item(int index) {
    return &list_of_connections[index];
}


int  list_of_connections_add(int sockfd) {
    int i = 0;

    while (i < MAX_NUMBER_OF_CONNECTED_USERS && list_of_connections[i].fd != -1) {
        i++;
    }

    if (i >= MAX_NUMBER_OF_CONNECTED_USERS) {
        return -1;
    }

    list_of_connections[i].fd = sockfd;
    return 1;
}


void list_of_connections_remove(int sockfd) {
    int user_index = get_user_index_(sockfd);
    if (user_index != -1) {
        list_of_connections[user_index].fd = -1;
    }
}


int list_of_connections_number_of_sockets_with_data(void) {
    return poll(list_of_connections, (nfds_t) MAX_NUMBER_OF_CONNECTED_USERS, 0);
}