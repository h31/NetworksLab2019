#ifndef SERVER_LINUX_INET_UTILS_H
#define SERVER_LINUX_INET_UTILS_H

#include <netinet/in.h>
#include <strings.h>

typedef struct sockaddr_in socket_descriptor;
typedef struct sockaddr address;
typedef struct hostent host_description;

void set_servsockdesc(socket_descriptor *serv_addr, uint16_t port);

void set_clientsockdesc(socket_descriptor *serv_addr, uint16_t port, in_addr_t *in_addr);

int create_tcpsocket();

#endif
