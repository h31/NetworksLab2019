#include <netdb.h>
#include <sys/filio.h>
#include <sys/ioctl.h>
#include "headers/inet_utils.h"
#include "../common-utils/headers/errors.h"

void set_sockdescriptor(socket_descriptor *serv_addr, uint16_t port, in_addr_t addr) {
    memset(serv_addr, 0, sizeof(socket_descriptor));

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = addr;
    serv_addr->sin_port = htons(port);
}

void set_clientsockdesc(socket_descriptor *serv_addr, uint16_t port, in_addr_t *in_addr) {
    set_sockdescriptor(serv_addr, port, *in_addr);
}

void set_servsockdesc(socket_descriptor *serv_addr, uint16_t port) {
    set_sockdescriptor(serv_addr, port, INADDR_ANY);
}

int create_socket(int domain, int socktype, int protocol) {
    int desc = socket(domain, socktype, protocol);
    if (desc < 0) raise_error(SOCK_OPEN_ERROR);
    return desc;
}

int create_tcpsocket() {
    return create_socket(AF_INET, SOCK_STREAM, 0);
}

int async_socket(int sock_descriptor) {
    int on = 1;
    int exit_code;
    exit_code = setsockopt(sock_descriptor, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    if (exit_code < 0) raise_error(SOCK_OPT);
    exit_code = ioctl(sock_descriptor, FIONBIO, (char *) &on);
    if (exit_code < 0) raise_error(ASYNC_SOCK);
    return exit_code;
}