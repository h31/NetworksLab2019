#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include "../common-utils/headers/common.h"
#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"
#include "../common-utils/headers/io.h"

void start_client(int argc, char *argv[]) {
    socket_descriptor serv_addr;

    uint16_t portno = exclude_cliport(argc, argv);
    int sockfd = create_tcpsocket();
    host_description *server = gethostbyname(argv[1]);

    if (server == NULL) {
        raise_error(NO_SUCH_HOST);
    }

    in_addr_t *in_addr = bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    set_clientsockdesc(&serv_addr, portno, in_addr);

    /* Now connect to the server */
    if (connect(sockfd, (address *) &serv_addr, sizeof(serv_addr)) < 0) {
        raise_error(CONNECT_ERROR);
    }

    printf("Please enter your name: ");
    char client_name[CLIENT_NAME_SIZE];
    bzero(client_name, CLIENT_NAME_SIZE);
    fgets(client_name, CLIENT_NAME_SIZE, stdin);

    /* firstly send client name */
    send_message(sockfd, client_name);
    for (;;) {}
}

int main(int argc, char *argv[]) {
    start_client(argc, argv);
}