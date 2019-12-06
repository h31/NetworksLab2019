#include <netinet/in.h>
#include <string.h>

#include "./list_of_clients.h"

#define LIST_OF_CLIENTS_SIZE 1024

Client clients[LIST_OF_CLIENTS_SIZE];


void list_of_clients__init() {
    for (int i = 0; i < LIST_OF_CLIENTS_SIZE; ++i) {
        bzero(&clients[i].addr, sizeof(clients[i].addr));
        clients[i].fd = NULL;
        clients[i].mode = -1;
        clients[i].block_number = -1;
    }
}


static int find_empty_place_() {
    int user_index;

    for (user_index = 0; clients[user_index].mode != -1; user_index++) {
        if (user_index == LIST_OF_CLIENTS_SIZE) {
            return -1;
        }
    }

    return user_index;
}


static int find_user_index_(struct sockaddr_in addr) {
    int user_index;

    for (user_index = 0; clients[user_index].addr.sin_port != addr.sin_port &&
            clients[user_index].addr.sin_addr.s_addr != addr.sin_addr.s_addr ; ++user_index) {
        if (user_index == LIST_OF_CLIENTS_SIZE) {
            return -1;
        }
    }

    return user_index;
}


int list_of_clients__add_client(struct sockaddr_in addr, FILE* fd, int mode, int block_number) {
    int user_index = find_empty_place_();

    if (user_index == -1) {
        return -1;
    }

    clients[user_index].addr = addr;
    clients[user_index].fd = fd;
    clients[user_index].mode = mode;
    clients[user_index].block_number = block_number;

    return 1;
}


void list_of_clients__remove_client(struct sockaddr_in addr) {
    int user_index = find_user_index_(addr);

    if (user_index != -1) {
        clients[user_index].fd = NULL;
        clients[user_index].mode = -1;
        clients[user_index].block_number = -1;
        bzero(&clients[user_index].addr, sizeof(clients[user_index].addr));
    }
}


int list_of_clients__set_block_number(struct sockaddr_in addr, int block_number) {
    int user_index = find_user_index_(addr);

    if (user_index == -1) {
        return -1;
    }

    clients[user_index].block_number = block_number;
    return 1;
}


int list_of_clients__client_exists(struct sockaddr_in addr) {
    return find_user_index_(addr) == -1 ? -1 : 1;
}


int list_of_clients__get_mode(struct sockaddr_in addr) {
    int user_index = find_user_index_(addr);
    return user_index == -1 ? -1 : clients[user_index].mode;
}

FILE* list_of_clients__get_file(struct sockaddr_in addr) {
    int user_index = find_user_index_(addr);
    return user_index == -1 ? NULL : clients[user_index].fd;
}


int list_of_clients__get_block_number(struct sockaddr_in addr) {
    int user_index = find_user_index_(addr);
    return user_index == -1 ? -1 : clients[user_index].block_number;
}