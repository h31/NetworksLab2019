#ifndef SERVER_LINUX_LIST_OF_USERS_H
#define SERVER_LINUX_LIST_OF_USERS_H

#include "../constants.h"

typedef struct {
    int sockfd;
    char name[MAX_NICKNAME_SIZE];
    int current_message_length;
    char current_message[MAX_RECEIVED_MESSAGE_SIZE];
} User_list_structure;

void list_of_users_init(void);

int list_of_users_add(int sockfd);

void list_of_users_remove(int sockfd);

char* list_of_users_get_name(int index);

char* list_of_users_get_message(int index);

int list_of_users_get_message_length(int index);

int list_of_users_get_socket_number(int index);

void list_of_users_add_data(int sockfd, void* data);

void list_of_users_clean_message(int index);

#endif //SERVER_LINUX_LIST_OF_USERS_H