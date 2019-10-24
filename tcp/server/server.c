#include <unistd.h>

#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"

void start_server(const uint16_t *port) {

    // first call to socket function
    int sockfd = create_tcpsocket();

    // initialize socket structure
    struct sockaddr_in serv_addr;
    set_sockdescriptor(&serv_addr, *port);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        raise_error(BINDING_ERROR);
    }

    if (listen(sockfd, 7) < 0) {
        raise_error(SOCKET_STATE_ERROR);
    }

    struct sockaddr_in client_addr;
    /* Accept actual connection from the client */

    socklen_t clilen = sizeof(client_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &clilen);

    char buffer[256];

    /* If connection is established then start communicating */
    bzero(buffer, 256);
    ssize_t n = read(newsockfd, buffer, 255); // recv on Windows

    if (n < 0) raise_error(SOCKET_READ_ERROR);


    printf("Here is the message: %s\n", buffer);

    /* Write a response to the client */
    n = write(newsockfd, "I got your message", 18); // send on Windows

    if (n < 0) raise_error(SOCKET_WRITE_ERROR);
}