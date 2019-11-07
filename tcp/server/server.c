#include <unistd.h>
#include <pthread.h>

#include "../common-utils/headers/io.h"
#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"
#include "../common-utils/headers/list.h"

#define MAX_QUEUED_CLIENTS 7

/* global vars between server functions */
pthread_mutex_t *locker = NULL;
List *cache = NULL;

void start_communication(Client *client) {
    while (!client->is_disconnected) {
        Reader *reader = read_message(client);
        if (reader->exit_code == SUCCESS) {
            foreach(&send_message, reader->value, cache, locker);
        }
        free_reader(reader);
    }
    printf("<logger>: client with name: %s left the chat", client->name);
    delete(cache, client, locker);
    free_client(client);
}

void start_client_scenario(void *args) {
    int *client_id = (int *) args;

    Reader *reader = read_clientname(*client_id);
    if (reader->exit_code == FAILURE) {
        free_reader(reader);
        raise_error(SOCKET_READ_ERROR);
    }

    /* copying client_name is necessary for reader deallocating */
    size_t len = strlen(reader->value);
    char *client_name = allocate_char_buffer(len);
    bcopy(reader->value, client_name, len);
    Client *client = new_client(client_id, client_name);

    free_reader(reader);

    /* apply client to cache */
    push(cache, client, locker);

    // TODO ???
    printf("<logger>: client with name %s is connected", client->name);
    // TODO ???
    char *welcome_msg = strcat(client_name, ": joined to the chat!");
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
    set_servsockdesc(&serv_addr, *port);

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