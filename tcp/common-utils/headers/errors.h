#ifndef SERVER_ERRORS_H
#define SERVER_ERRORS_H

#include "stdio.h"
#include <stdlib.h>

#define WRONG_PORT_FORMAT "can't parse port from argv"
#define WRONG_ARGS_NUMBER "args number should be equal 2"
#define SOCK_OPEN_ERROR "error opening socket"
#define BINDING_ERROR "error binding socket to ip addr and port"
#define SOCKET_STATE_ERROR "can't put socket to listeningg state"
#define ACCEPT_ERROR "can't accept new client"
#define THREAD_ERROR "can't allocate a thread for a client"
#define MUTEX_INIT_ERROR "can't initialize mutex"
#define SOCKET_READ_ERROR "can't read from socket"
#define SOCKET_WRITE_ERROR "can't write to socket"
#define NO_SUCH_ELEMENT "no such element in a List"
#define NO_SUCH_HOST "no such host"
#define CONNECT_ERROR "can't connect"

void raise_error(char *error);

#endif //SERVER_ERRORS_H