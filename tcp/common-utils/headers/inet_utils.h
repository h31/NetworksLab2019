#ifndef SERVER_LINUX_INET_UTILS_H
#define SERVER_LINUX_INET_UTILS_H

#include <netinet/in.h>
#include <strings.h>
#include <poll.h>

typedef struct sockaddr_in socket_descriptor;
typedef struct sockaddr address;
typedef struct hostent host_description;
typedef struct pollfd poll_descriptor;

typedef struct Poll_vector {
    nfds_t length;
    poll_descriptor *descriptors;
} Poll_vector;

void set_servsockdesc(socket_descriptor *serv_addr, uint16_t port);

void set_clientsockdesc(socket_descriptor *serv_addr, uint16_t port, in_addr_t *in_addr);

int create_tcpsocket();

int async_socket(int sock_descriptor);

#endif
