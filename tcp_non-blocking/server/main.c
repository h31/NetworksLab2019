#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include "list_of_users/list_of_users.h"
#include "list_of_connections/list_of_connections.h"
#include "message_sending_handler/messages_sending_handler.h"
#include "message_receiving_handler/message_receiving_handler.h"


// checking number of arguments
void checkArguments_(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage %s port \n", argv[0]);
        exit(0);
    };
}


void add_user_info_(int user_sockfd) {
    if (user_sockfd > 0) {
        list_of_connections_add(user_sockfd);
        list_of_users_add(user_sockfd);
    }
}


// make socket address reusable
void make_socket_addr_reusable_(int sockfd) {
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        fprintf(stderr, "ERROR: setsockopt(SO_REUSEADDR) failed");
    }
}

// bind the host address using bind() call.
void bind_socket_(int sockfd, struct sockaddr_in serv_addr) {
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding\n");
        exit(1);
    }
}

// Initialize initial_socket structure
void init_socket_structure_(int port_number, struct sockaddr_in* serv_addr) {
    bzero((char *) serv_addr, sizeof(*serv_addr));
    serv_addr -> sin_family = AF_INET;
    serv_addr -> sin_addr.s_addr = INADDR_ANY;
    serv_addr -> sin_port = htons(port_number);
}


void setup_initial_socket_(int* initial_socket, uint16_t port_number) {
    struct sockaddr_in serv_addr;

    *initial_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (*initial_socket < 0) {
        fprintf(stderr, "ERROR opening init socket\n");
        exit(1);
    }

    make_socket_addr_reusable_(*initial_socket);

    init_socket_structure_(port_number, &serv_addr);

    bind_socket_(*initial_socket, serv_addr);
}


int main(int argc, char* argv[]) {
    int initial_socket;
    unsigned int clilen;
    struct sockaddr_in cli_addr;

    checkArguments_(argc, argv);

    const uint16_t port_number = (uint16_t)  atoi(argv[1]);

    fprintf(stdout, "server start at port: %d\n", port_number);

    setup_initial_socket_(&initial_socket, port_number);

    listen(initial_socket, 5);
    clilen = sizeof(cli_addr);

    list_of_connections_init();
    list_of_users_init();

    // чтобы не крашиться при попытке записать в закрытый сокет
    signal(SIGPIPE, SIG_IGN);

    list_of_connections_add(initial_socket);

    while(1) {

        if (list_of_connections_is_initial_socket_not_empty()) {
            add_user_info_( accept(list_of_connections_get_item(0)->fd, (struct sockaddr *) &cli_addr, &clilen) );
        }

        message_receiving_handler_read_messages();

        message_sending_handler_send_messages();
    }
}