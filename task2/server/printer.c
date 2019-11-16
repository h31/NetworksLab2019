//
// Created by Malik Hiraev on 10/11/2019.
//

#include "printer.h"

void print_client_name_and_address(Client *client) {
    __uint32_t cli_addr = client->sockaddr->sin_addr.s_addr;
    printf(
            "%s (%d-%d.%d.%d.%d)",
            client->name,
            client->fd_index,
            (u_char) cli_addr,
            (u_char) (cli_addr >> 8),
            (u_char) (cli_addr >> 16),
            (u_char) (cli_addr >> 24)
    );
}

void print_client_disconnected(Client *client) {
    print_client_name_and_address(client);
    printf(" disconnected\n");
}

void print_client_connected(Client *client) {
    print_client_name_and_address(client);
    printf(" connected\n");
}
