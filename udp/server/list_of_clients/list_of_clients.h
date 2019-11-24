
#ifndef UDP_SERVER_LINUX_LIST_OF_CLIENTS_H
#define UDP_SERVER_LINUX_LIST_OF_CLIENTS_H

#include <netinet/in.h>
#include <stdio.h>

typedef struct {
    struct sockaddr_in addr;
    int mode;
    FILE* fd;
    int block_number;
} Client;

void list_of_clients__init();

int list_of_clients__client_exists(struct sockaddr_in addr);

int list_of_clients__add_client(struct sockaddr_in addr, FILE* fd, int mode, int block_number);

void list_of_clients__remove_client(struct sockaddr_in addr);

int list_of_clients__set_block_number(struct sockaddr_in addr, int block_number);

int list_of_clients__get_mode(struct sockaddr_in addr);

FILE* list_of_clients__get_file(struct sockaddr_in addr);

int list_of_clients__get_block_number(struct sockaddr_in addr);

#endif //UDP_SERVER_LINUX_LIST_OF_CLIENTS_H
