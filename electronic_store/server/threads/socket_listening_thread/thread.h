
#ifndef ELECTRONIC_STORE_SL_THREAD_H
#define ELECTRONIC_STORE_SL_THREAD_H

typedef struct {
    int sockfd;
} Listening_thread_input;

pthread_t create_listening_thread(int sockfd);

#endif //ELECTRONIC_STORE_THREAD_H
