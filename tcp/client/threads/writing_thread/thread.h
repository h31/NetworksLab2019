
#ifndef CLIENT_LINUX_WTHREAD_H
#define CLIENT_LINUX_WTHREAD_H


#include <ncurses.h>
#include <pthread.h>

typedef struct writing_thread_input {
    int sockfd;
    int input_line_size;
} Writing_thread_input;

pthread_t create_writing_thread(int sockfd, int input_line_size);

#endif //CLIENT_LINUX_THREAD_H
