
#ifndef SERVER_LINUX_USER_INFO_LIST_H
#define SERVER_LINUX_USER_INFO_LIST_H

#include <pthread.h>

pthread_mutex_t user_info_mutex;

typedef struct user_info {
    int client_socket;
    char *nickname;
    struct user_info *next;
} User_info;

extern User_info *user_info_head_element;

User_info *user_info_init_new_item(char *nickname, int socket);

void user_info_add(User_info *new_element);

void user_info_remove_by_socket(int client_socket);

void user_info_init_mutex(void);

int user_info_get_socket_number(User_info *element);

char* user_info_get_nickname_by_socket(int sockfd);

User_info *user_info_get_next(User_info *element);

#endif //SERVER_LINUX_USER_INFO_LIST_H
