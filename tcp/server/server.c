#include <unistd.h>

#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"

#define MAX_QUEUED_CLIENTS 7

void listen_cli(int sockfd);

void start_server(const uint16_t *port) {
    int sockfd = create_tcpsocket();

    socket_descriptor serv_addr;
    set_sockdescriptor(&serv_addr, *port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        raise_error(BINDING_ERROR);
    }

    if (listen(sockfd, MAX_QUEUED_CLIENTS) < 0) {
        raise_error(SOCKET_STATE_ERROR);
    }

    for (;;) {
        listen_cli(sockfd);
    }
}

void listen_cli(int sockfd) {
    socket_descriptor cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    int newsockfd = accept(sockfd, (struct sockaddr *) &cliaddr, &clilen);
}