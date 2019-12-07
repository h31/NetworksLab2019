
#ifndef ELECTRONIC_STORE_LIST_OF_CLIENTS_H
#define ELECTRONIC_STORE_LIST_OF_CLIENTS_H

typedef struct user_info {
    int sockfd;
    struct user_info* next;
} User_info;

User_info* list_of_clients_make_new_item(int sockfd);

void list_of_clients_add(User_info* new_element);

void list_of_clients_export(FILE* dst_fd);

int list_of_clients_remove(int sockfd);

void list_of_clients_remove_all(void);

void init_list_of_clients_mutex(void);

#endif //ELECTRONIC_STORE_LIST_OF_CLIENTS_H
