
#ifndef ELECTRONIC_STORE_AC_THREAD_H
#define ELECTRONIC_STORE_AC_THREAD_H

typedef struct accepting_thread_input {
    int port_number;
    int initial_sockfd;
} Accepting_thread_input;

pthread_t create_accepting_thread(int port_number, int initial_sockfd);

#endif //ELECTRONIC_STORE_THREAD_H
