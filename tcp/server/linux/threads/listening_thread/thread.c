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


int readn(int sockfd, void* dst, size_t len) {
    int total_number_read = 0;
    int local_number_read = 0;

    while (len > 0) {
        local_number_read = read(sockfd, dst, len);

        if (local_number_read == 0) {
            return total_number_read;
        }

        if (local_number_read < 0) {
            return -1;
        }

        total_number_read += local_number_read;
        len -= local_number_read;
    }

    return total_number_read;
}


void add_date_to_message(char* dst) {
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    char time[TIME_SIZE + 1];
    strftime (time, TIME_SIZE + 1, "<%H:%M:%S>", timeinfo);

    strncat(dst, time, TIME_SIZE);
}


void add_nickname_to_message(char* dst, int sockfd) {
    strncat(dst, user_info_get_nickname_by_socket(sockfd), NICKNAME_SIZE);
    char* r = ": ";
    strncat(dst, r, strlen(r));
}


char* read_user_nickname(int sockfd) {
    char buffer[L_THREAD_MESSAGE_TEXT];

    bzero(buffer, L_THREAD_MESSAGE_TEXT);
    int tmp ;
    // read message size
    readn(sockfd, (char*) &tmp, sizeof(int));
    // read message
    if (readn(sockfd, buffer, tmp) < 0) {
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

        int tmp;
        readn(((Listening_thread_input*) arg) -> sockfd, &tmp, sizeof(int));
        number_read = readn(((Listening_thread_input*) arg) -> sockfd, message_text, tmp);

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
            add_date_to_message(full_message);
            add_nickname_to_message(full_message, ((Listening_thread_input*) arg) -> sockfd);
            strncat(full_message, message_text, L_THREAD_MESSAGE_TEXT);

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
