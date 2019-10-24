#ifndef SERVER_ERRORS_H
#define SERVER_ERRORS_H

#include "stdio.h"
#include <stdlib.h>

#define WRONG_PORT_FORMAT "can't parse port from argv"
#define WRONG_ARGS_NUMBER "args number should be equal 2"
#define SOCK_OPEN_ERROR "error opening socket"
#define BINDING_ERROR "error binding socket to ip addr and port"
#define SOCKET_STATE_ERROR "can't put socket to listeningg state"
#define SOCKET_READ_ERROR "can't read from socket"
#define SOCKET_WRITE_ERROR "can't write to socket"

void raise_error(char *error);

#endif //SERVER_ERRORS_H