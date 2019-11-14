
#ifndef SERVER_LINUX_LIST_OF_CONNECTIONS_H
#define SERVER_LINUX_LIST_OF_CONNECTIONS_H

void list_of_connections_init(void);

int  list_of_connections_add(int sockfd);

void list_of_connections_remove(int sockfd);

int list_of_connections_number_of_sockets_with_data(void);

struct pollfd* list_of_connections_get_item(int index);

int list_of_connections_is_initial_socket_not_empty(void);

#endif //SERVER_LINUX_LIST_OF_CONNECTIONS_H