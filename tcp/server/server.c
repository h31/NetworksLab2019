#include <unistd.h>
#include <pthread.h>

#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"
#include "../common-utils/headers/list.h"

#define MAX_QUEUED_CLIENTS 7

void wait_client(int sockfd);

void allocate_thread();

void start_server(const uint16_t *port) {
    // govno-code :(
    List *cache = (List *) malloc(sizeof(List));
    *cache = (struct List) {0, NULL};
    pthread_mutex_t locker;
    if (pthread_mutex_init(&locker, NULL) != 0) {
        raise_error(MUTEX_INIT_ERROR);
    }

    int sockfd = create_tcpsocket();

    socket_descriptor serv_addr;
    set_sockdescriptor(&serv_addr, *port);

    if (bind(sockfd, (address *) &serv_addr, sizeof(serv_addr)) < 0) {
        raise_error(BINDING_ERROR);
    }

    if (listen(sockfd, MAX_QUEUED_CLIENTS) < 0) {
        raise_error(SOCKET_STATE_ERROR);
    }

    for (;;) {
        wait_client(sockfd);
    }
}

void wait_client(int sockfd) {
    socket_descriptor client_addr;
    socklen_t clilen = sizeof(client_addr);

    int newsockfd = accept(sockfd, (address *) &client_addr, &clilen);
    if (newsockfd < 0) raise_error(ACCEPT_ERROR);
    else allocate_thread();
}

void allocate_thread() {
    pthread_t client_thread;
    int status;
    status = pthread_create(&client_thread, NULL, ???, NULL); // TODO
    if (status != 0) raise_error(THREAD_ERROR);
}