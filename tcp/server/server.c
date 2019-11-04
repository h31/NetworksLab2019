#include <unistd.h>
#include <pthread.h>

#include "io.h"
#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"
#include "../common-utils/headers/list.h"

#define MAX_QUEUED_CLIENTS 7

/* global vars between server functions */
pthread_mutex_t *locker = NULL;
List *cache = NULL;

void start_communication(Client *client) {
    while (!client->is_disconnected) {
        char msg[MSG_SIZE];
        char *message = read_message(client, msg);
        if (strlen(message) != FAILURE) { // TODO govno_code
            foreach(&send_message, message, cache, locker);
        }
    }
}

void start_client_scenario(void *args) {
    int *client_id = (int *) args;

    /*
     * It is neccesary to define buffer here, because
     * we can't define and return pointer to local var's
     */
    char name_buffer[CLIENT_NAME_SIZE];

    char *name = read_clientname(*client_id, name_buffer);
    Client *client = new_client(client_id, name);

    /* apply client to cache */
    push(cache, client, locker);

    char *welcome_msg = strcat(name, ": joined to the chat!");
    foreach(&send_message, welcome_msg, cache, locker);

    start_communication(client);
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