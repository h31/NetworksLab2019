
#ifndef CLIENT_LINUX_LTHREAD_H
#define CLIENT_LINUX_LTHREAD_H

#include <ncurses.h>
typedef struct listening_thread_input {
    int sockfd;
    int output_window_height;
} Listening_thread_input;

pthread_t create_listening_thread(int sockfd, int output_window_height);
#endif //CLIENT_LINUX_THREAD_H
