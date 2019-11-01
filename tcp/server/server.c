#include <unistd.h>
#include <pthread.h>

#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"
#include "../common-utils/headers/list.h"

#define MAX_QUEUED_CLIENTS 7

/* global vars between server functions */
pthread_mutex_t *locker = NULL;
List *cache = NULL;

void start_client_scenario(void *args) {

    // TODO get client name
    Client *client = allocate_client(*((int *) args), ???);

    /* apply client to cache */
    push(cache, client, locker);
    // TODO call read msgs
}

void allocate_thread(int *client_id) {
    pthread_t client_thread;
    int status;
    status = pthread_create(&client_thread, NULL, (void *) &start_client_scenario, client_id);
    if (status != 0) raise_error(THREAD_ERROR);
}

void wait_client(int sockfd) {
    socket_descriptor client_addr;
    socklen_t clilen = sizeof(client_addr);

    int newsockfd = accept(sockfd, (address *) &client_addr, &clilen);
    if (newsockfd < 0) raise_error(ACCEPT_ERROR);
    else allocate_thread(&newsockfd);
}

void start_server(const uint16_t *port) {
    /* initialize global variabels */
    Env *env = init_env();
    locker = env->blocker;
    cache = env->cache;

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