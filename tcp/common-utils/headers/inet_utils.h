#ifndef SERVER_LINUX_INET_UTILS_H
#define SERVER_LINUX_INET_UTILS_H

#include <netinet/in.h>
#include <strings.h>

typedef struct sockaddr_in socket_descriptor;

void set_sockdescriptor(socket_descriptor *serv_addr, uint16_t port);

#endif
