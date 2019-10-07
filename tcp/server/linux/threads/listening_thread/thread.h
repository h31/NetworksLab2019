
#ifndef SERVER_LINUX_LTHREAD_H
#define SERVER_LINUX_LTHREAD_H

#define L_THREAD_MESSAGE_TEXT 256

typedef struct listening_thread_input {
    int sockfd;
} Listening_thread_input;

void *listening_thread(void *arg);

void create_listening_thread(int sockfd);

Listening_thread_input* listening_thread_init_input_structure(int sockfd);

#endif //SERVER_LINUX_THREAD_H
