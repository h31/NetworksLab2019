#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "thread.h"
#include "../../message_buffer/message_buffer.h"
#include "../../user_info_list/user_info_list.h"

#define TIME_SIZE 10
#define NICKNAME_SIZE 10


void add_date_to_message(char* dst) {
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    char time[TIME_SIZE + 1];
    strftime (time, TIME_SIZE + 1, "<%H:%M:%S>", timeinfo);

    strcat(dst, time);
}


void add_nickname_to_message(char* dst, int sockfd) {
    strcat(dst, user_info_get_nickname_by_socket(sockfd));
    char* r = ": ";
    strcat(dst, r);
}


char* read_user_nickname(int sockfd) {
    char buffer[L_THREAD_MESSAGE_TEXT];

    bzero(buffer, L_THREAD_MESSAGE_TEXT);

    if (read(sockfd, buffer, L_THREAD_MESSAGE_TEXT - 1) < 0) {
        fprintf(stderr, "ERROR reading from socket (%d)\n", sockfd);
    }
    fprintf(stdout, "client connected. Username: %s, socket: %d\n", buffer, sockfd);

    return strdup(buffer);
}


// listening socket for input message
void *listening_thread(void *arg) {
    char message_text[L_THREAD_MESSAGE_TEXT];
    char full_message[TIME_SIZE + NICKNAME_SIZE + L_THREAD_MESSAGE_TEXT];
    int number_read;

    // get user nickname and add user to list
    user_info_add(
            user_info_init_new_item(
                    read_user_nickname(((Listening_thread_input*) arg) -> sockfd),
                    ((Listening_thread_input*) arg) -> sockfd)
    );

    while (1) {
        bzero(message_text, L_THREAD_MESSAGE_TEXT);
        bzero(full_message, TIME_SIZE + NICKNAME_SIZE + L_THREAD_MESSAGE_TEXT);

        number_read = read(((Listening_thread_input*) arg) -> sockfd, message_text, L_THREAD_MESSAGE_TEXT - 1);

        if (number_read < 0) {
            fprintf(stderr, "ERROR reading from socket (%d)\n", ((Listening_thread_input*) arg) -> sockfd);
        }


        if (number_read == 0) {
            user_info_remove_by_socket( ((Listening_thread_input*) arg) -> sockfd );
            shutdown( ((Listening_thread_input*) arg) -> sockfd, SHUT_RDWR);
            close( ((Listening_thread_input*) arg) -> sockfd );
            fprintf(stdout, "client disconnected: %d\n", ((Listening_thread_input*) arg) -> sockfd);
            pthread_exit(0);
        }

        if (number_read > 0) {
            //message_buffer_put(buffer);
            add_date_to_message(full_message);
            add_nickname_to_message(full_message, ((Listening_thread_input*) arg) -> sockfd);
            strcat(full_message, message_text);

            while (!message_buffer_put(full_message)) {
            }
            printf("put to buffer: %s\n", full_message);
        }
    }

}


void create_listening_thread(int sockfd) {
    pthread_t listnening_thread;
    Listening_thread_input* listening_thread_input = listening_thread_init_input_structure(sockfd);

    if( pthread_create(&listnening_thread, NULL, listening_thread, (void *) listening_thread_input)) {
        fprintf(stderr, "ERROR creating listening thread\n");
    }
}


Listening_thread_input* listening_thread_init_input_structure(int sockfd) {
    Listening_thread_input* new_input_structure = (Listening_thread_input*) malloc(sizeof(Listening_thread_input));

    new_input_structure -> sockfd = sockfd;

    return new_input_structure;
}
