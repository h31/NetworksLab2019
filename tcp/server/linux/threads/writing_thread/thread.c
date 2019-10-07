#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "thread.h"
#include "../../message_buffer/message_buffer.h"


// write messages to users
void *writing_thread(void *arg) {

    int number_written;
    char* message;

    while (1) {

        message = message_buffer_poll();

        if (message != NULL) {
            User_info *cur = user_info_head_element;

            while (cur != NULL) {
                // узнаём сокет
                int sockfd = user_info_get_socket_number(cur);
                //пишем в сокет
                number_written = write(sockfd, message, strlen(message) + 1);

                if (number_written < 0) {
                    fprintf(stderr, "ERROR write message to client thread\n");
                } else {
                    printf("write to socket: %d\n", sockfd);
                }
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
