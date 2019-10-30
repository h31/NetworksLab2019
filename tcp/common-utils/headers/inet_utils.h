#ifndef SERVER_LINUX_INET_UTILS_H
#define SERVER_LINUX_INET_UTILS_H

#include <netinet/in.h>
#include <strings.h>

typedef struct sockaddr_in socket_descriptor;
typedef struct sockaddr address;

void set_sockdescriptor(socket_descriptor *serv_addr, uint16_t port);

int create_tcpsocket();

#endif
