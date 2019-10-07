#include "user_info_list.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

User_info* user_info_head_element = NULL;


//list element structure
User_info* user_info_init_new_item(char* nickname, int socket) {
    User_info* new_item = (User_info*) malloc(sizeof(User_info));
    new_item -> nickname = nickname;
    new_item -> next = NULL;
    new_item -> client_socket = socket;
    return new_item;
}


// add given element to list
void user_info_add(User_info* new_element) {
    pthread_mutex_lock(&user_info_mutex);

    // create local pointer to save pointer to head
    User_info* local_pointer = user_info_head_element;

    if (local_pointer == NULL) {
        user_info_head_element = new_element;
        pthread_mutex_unlock(&user_info_mutex);
        return;
    }

    while (local_pointer -> next != NULL) {
        local_pointer = local_pointer -> next;
    }

    local_pointer -> next = new_element;

    pthread_mutex_unlock(&user_info_mutex);
}


void user_info_remove_by_socket(int client_socket) {
    pthread_mutex_lock(&user_info_mutex);

    User_info* tmp_element = user_info_head_element;
    User_info* prev = NULL;

    while (tmp_element -> client_socket != client_socket && tmp_element -> next != NULL) {
        prev = tmp_element;
        tmp_element = tmp_element -> next;
    }

    if (tmp_element -> client_socket == client_socket) {
        if (prev == NULL) {
            user_info_head_element = user_info_head_element->next;
        } else if (tmp_element -> next == NULL) {
            prev -> next = NULL;
        } else {
            prev -> next = tmp_element -> next;
        }
        free(tmp_element);
    }

    pthread_mutex_unlock(&user_info_mutex);
}


char* user_info_get_nickname_by_socket(int sockfd) {
    char* result;
    pthread_mutex_lock(&user_info_mutex);

    User_info* tmp_element = user_info_head_element;

    while (tmp_element->client_socket != sockfd) {
        tmp_element = tmp_element->next;
    }
    result = strdup(tmp_element->nickname);

    pthread_mutex_unlock(&user_info_mutex);

    return result;
}

// get socket number by pointer to structure
int user_info_get_socket_number(User_info* element) {
    int result;
    pthread_mutex_lock(&user_info_mutex);
    result = element -> client_socket;
    pthread_mutex_unlock(&user_info_mutex);
    return result;
}


// get next element of the given
User_info* user_info_get_next(User_info* element) {
    User_info* result;
    pthread_mutex_lock(&user_info_mutex);
    result = element -> next;
    pthread_mutex_unlock(&user_info_mutex);
    return result;
}


void user_info_init_mutex() {
    pthread_mutex_init(&user_info_mutex, NULL);
}