#include "headers/inet_utils.h"

void set_sockdescriptor(socket_descriptor *serv_addr, uint16_t port) {
    memset(serv_addr, 0, sizeof(socket_descriptor)); // TODO

    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = INADDR_ANY;
    serv_addr->sin_port = htons(port);
}