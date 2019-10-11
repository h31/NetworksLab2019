#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "thread.h"
#include "../../message_buffer/message_buffer.h"


void send_message_to_client(int sockfd, char * message, size_t size) {
    // sending message size
    int message_size = (int) strlen(message);
    if (write(sockfd, &message_size, sizeof(message_size)) < 0) {
        fprintf(stderr, "ERROR writing to socket\n");
    }
    // sending message
    if (write(sockfd, message, size) < 0) {
        fprintf(stderr, "ERROR writing to socket\n");
    }
}


// write messages to users
void *writing_thread(void *arg) {
    char* message;

    while (1) {
        message = message_buffer_poll();

        if (message != NULL) {
            User_info *cur = user_info_head_element;

            while (cur != NULL) {
                // узнаём сокет
                int sockfd = user_info_get_socket_number(cur);
                //пишем в сокет
                send_message_to_client(sockfd, message, strlen(message));

                //переходим к следующему элементу
                cur = user_info_get_next(cur);
            }

        }
    }

}


void create_writing_thread() {
    pthread_t _writing_thread;

    if( pthread_create(&_writing_thread, NULL, writing_thread, NULL)) {
        fprintf(stderr, "ERROR creating listening thread\n");
    }
}
