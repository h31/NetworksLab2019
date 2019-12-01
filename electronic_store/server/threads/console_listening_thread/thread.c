#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "../../list_of_clients/list_of_clients.h"
#include "./thread.h"

void* console_listening_thread(void* arg) {
    char user_command[MAX_USER_COMMAND_SIZE];

    while(1) {

        bzero(user_command, MAX_USER_COMMAND_SIZE);
        char* command = fgets(user_command, MAX_USER_COMMAND_SIZE, stdin);

        if (strcmp(command, "show clients\n") == 0) {
            list_of_clients_export(stdout);

        } else if (strcmp(command, "disconnect client\n") == 0) {
            int sockfd;
            printf("Enter client socket number: ");
            fflush(stdout);
            scanf("%d", &sockfd);
            if (list_of_clients_remove(sockfd) == -1) {
                printf("Error: no such user socket number.\n");
            } else {
                printf("User was successfully disconnected.\n");
            }

        } else if (strcmp(command, "quite\n") == 0) {
            shutdown(*(int*) arg, SHUT_RDWR);
            close(*(int*) arg);
            pthread_exit(0);
        }
    }
}


pthread_t create_user_listening_thread(int* initial_socket) {
    pthread_t _user_listening_thread;

    if ( pthread_create(&_user_listening_thread, NULL, console_listening_thread, (void*) initial_socket)) {
        return -1;
    }

    return _user_listening_thread;
}