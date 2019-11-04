#include <string.h>
#include <stdlib.h>
#include "list_of_users.h"
#include "../constants.h"

User_list_structure list_of_users[MAX_NUMBER_OF_CONNECTED_USERS];

static int get_user_index_(int sockfd) {
    int i = 0;
    while (i < MAX_NUMBER_OF_CONNECTED_USERS && list_of_users[i].sockfd != sockfd) {
        i++;
    }
    return (i < MAX_NUMBER_OF_CONNECTED_USERS) ? i : -1;
}


static int get_free_cell_() {
    int i = 0;
    while (i < MAX_NUMBER_OF_CONNECTED_USERS && list_of_users[i].sockfd != -1) {
        i++;
    }
    return (i < MAX_NUMBER_OF_CONNECTED_USERS) ? i : -1;
}


static void remove_user_info_(int user_index) {
    if (user_index > -1 && user_index < MAX_NUMBER_OF_CONNECTED_USERS) {
        list_of_users[user_index].sockfd = -1;
        list_of_users[user_index].current_message_length = -1;
        bzero(list_of_users[user_index].name, MAX_NICKNAME_SIZE);
        bzero(list_of_users[user_index].current_message, MAX_RECEIVED_MESSAGE_SIZE);
    }
}


static void shift_array_left_(void* array, int offset) {
    void *tmp = malloc(strlen(array));
    memcpy(tmp, array + offset, sizeof(tmp));
    bzero(array, sizeof(array));
    memcpy(array, tmp, sizeof(tmp));
    free(tmp);
}


static void array_append_(void* dst, int dst_size, void* src) {
    memcpy(dst + strnlen(dst, dst_size),
           src,
           dst_size - strnlen(dst, dst_size)
    );
}


void list_of_users_init(void) {
    for (int i = 0; i < MAX_NUMBER_OF_CONNECTED_USERS; i++) {
        list_of_users[i].sockfd = -1;
        bzero(list_of_users[i].name, MAX_NICKNAME_SIZE);
        bzero(list_of_users[i].current_message, MAX_RECEIVED_MESSAGE_SIZE);
        list_of_users[i].current_message_length = -1;
    }
}


int list_of_users_add(int sockfd) {
    int free_cell_index = get_free_cell_();

    if (free_cell_index != -1) {
        list_of_users[free_cell_index].sockfd = sockfd;
    }

    return free_cell_index;
}


void list_of_users_remove(int sockfd) {
    remove_user_info_(get_user_index_(sockfd));
}


int list_of_users_get_socket_number(int index) {
    return (index < MAX_NUMBER_OF_CONNECTED_USERS && index > -1) ? list_of_users[index].sockfd : -1;
}


char* list_of_users_get_name(int index) {
    return (index < MAX_NUMBER_OF_CONNECTED_USERS && index > -1) ?  list_of_users[index].name : NULL;
}


char* list_of_users_get_message(int index) {
    return (index < MAX_NUMBER_OF_CONNECTED_USERS && index > -1) ? list_of_users[index].current_message : NULL;
}


void list_of_users_clean_message(int index) {
    bzero(list_of_users[index].current_message, MAX_RECEIVED_MESSAGE_SIZE);
    list_of_users[index].current_message_length = -1;
}


// длина сообщения, которое нужно прочитать
int list_of_users_get_message_length(int index) {
    return (index < MAX_NUMBER_OF_CONNECTED_USERS && index > -1) ? list_of_users[index].current_message_length : -1;
}


static void set_message_length_(int user_index) {
    if (list_of_users_get_message_length(user_index) == -1) {
        // записываем длину принимаемого сообщения
        memcpy((void *) &list_of_users[user_index].current_message_length,
               (void *) list_of_users_get_message(user_index),
               sizeof(int));
        // смещаем принятые данные (убираем длину принимаемого сообщения)
        shift_array_left_(list_of_users[user_index].current_message, sizeof(int));
    }
}


static void set_nickname_(int user_index) {
    if (strnlen(list_of_users_get_name(user_index), MAX_NICKNAME_SIZE) == 0 &&
        strnlen(list_of_users_get_message(user_index), MAX_RECEIVED_MESSAGE_SIZE) != 0 ) {
        // записываем никнейм
        memcpy(list_of_users[user_index].name, list_of_users[user_index].current_message, MAX_NICKNAME_SIZE);
        // очищаем буфер
        bzero(list_of_users[user_index].current_message, MAX_RECEIVED_MESSAGE_SIZE);
        list_of_users[user_index].current_message_length = -1;
    }
}


void list_of_users_add_data(int sockfd, void* data) {
    int user_index = get_user_index_(sockfd);

    // добавляем прочитанные данные к уже принятым
    array_append_( list_of_users_get_message(user_index), MAX_RECEIVED_MESSAGE_SIZE, data );

    // записываем длину принимаемого сообщения
    set_message_length_(user_index);

    // записываем никнейм
    set_nickname_(user_index);
}