
#include <bits/types/FILE.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "./list_of_clients.h"
#include "../logger/logger.h"

static pthread_mutex_t list_of_clients_mutex_;
static User_info* head_element = NULL;


void init_list_of_clients_mutex() {
    pthread_mutex_init(&list_of_clients_mutex_, NULL);
}


User_info* list_of_clients_make_new_item(int sockfd) {
    User_info* new_item = (User_info*) malloc(sizeof(User_info));
    new_item -> sockfd = sockfd;
    new_item -> next = NULL;

    return new_item;
}


void list_of_clients_add(User_info* new_element) {
    pthread_mutex_lock(&list_of_clients_mutex_);

    if (head_element == NULL) {
        head_element = new_element;
    } else {
        User_info* iterator;
        for (iterator = head_element; iterator -> next != NULL; iterator = iterator -> next) {
        }
        iterator -> next = new_element;
    }

    pthread_mutex_unlock(&list_of_clients_mutex_);
    log_client_action("CLIENT", new_element->sockfd, "client connected");
}


int list_of_clients_remove(int sockfd) {
    pthread_mutex_lock(&list_of_clients_mutex_);

    if (head_element != NULL) {
        User_info* iterator = head_element;
        User_info* prev = NULL;

        while (iterator != NULL && iterator->sockfd != sockfd) {
         iterator = iterator->next;
         prev = iterator;
        }

        if (iterator == NULL) {
            pthread_mutex_unlock(&list_of_clients_mutex_);
            return -1;
        }

        if (prev == NULL) {
            shutdown(head_element -> sockfd, SHUT_RDWR);
            close(head_element -> sockfd);

            User_info* old_head = head_element;
            head_element = head_element -> next;
            free(old_head);

            pthread_mutex_unlock(&list_of_clients_mutex_);
            log_client_action("CLIENT", sockfd, "client disconnected");
            return 1;
        }

        prev -> next = iterator -> next;
        shutdown(iterator -> sockfd, SHUT_RDWR);
        close(iterator -> sockfd);
        free(iterator);

        pthread_mutex_unlock(&list_of_clients_mutex_);
        log_client_action("CLIENT", sockfd, "client disconnected");
        return 1;
    }

    pthread_mutex_unlock(&list_of_clients_mutex_);
    return -1;
}


void list_of_clients_export(FILE* dst_fd) {
    pthread_mutex_lock(&list_of_clients_mutex_);

    if (head_element == NULL) {
        fprintf(dst_fd, "No clients.\n");
        pthread_mutex_unlock(&list_of_clients_mutex_);
        return;
    }
    User_info* iterator = head_element;
    int index = 1;

    while (iterator != NULL) {
        fprintf(dst_fd, "%d) %d\n", index++, iterator->sockfd);
        iterator = iterator->next;
    }

    pthread_mutex_unlock(&list_of_clients_mutex_);
}


void list_of_clients_remove_all() {
    pthread_mutex_lock(&list_of_clients_mutex_);

    User_info* iterator = head_element;
    User_info* iterator_next;

    while(iterator != NULL) {
        iterator_next = iterator->next;

        shutdown(iterator -> sockfd, SHUT_RDWR);
        close(iterator -> sockfd);
        log_client_action("CLIENT", iterator->sockfd, "client disconnected");
        free(iterator);

        iterator = iterator_next;
    }

    pthread_mutex_unlock(&list_of_clients_mutex_);
}