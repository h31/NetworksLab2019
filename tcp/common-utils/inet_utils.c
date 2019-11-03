#include "headers/inet_utils.h"
#include "../common-utils/headers/errors.h"

void set_sockdescriptor(socket_descriptor *serv_addr, uint16_t port) {
    memset(serv_addr, 0, sizeof(socket_descriptor));

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = INADDR_ANY;
    serv_addr->sin_port = htons(port);
}

int create_socket(int domain, int socktype, int protocol) {
    int desc = socket(domain, socktype, protocol);
    if (desc < 0) raise_error(SOCK_OPEN_ERROR);
    return desc;
}

int create_tcpsocket() {
    return create_socket(AF_INET, SOCK_STREAM, 0);
}