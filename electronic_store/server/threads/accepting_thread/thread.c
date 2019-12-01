#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "../../list_of_clients/list_of_clients.h"
#include "../socket_listening_thread/thread.h"
#include "thread.h"

static void make_socket_reusable_(int sockfd) {
    // сделать адрес сокета многоразовым
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        fprintf(stderr, "ERROR: setsockopt(SO_REUSEADDR) failed\n");
    }
}


void fill_server_info_(struct sockaddr_in* serv_addr, int port_number) {
    // инициализация структуры initial_socket
    bzero((char *) serv_addr, sizeof(&serv_addr));
    serv_addr -> sin_port = htons(port_number);
    serv_addr -> sin_addr.s_addr = INADDR_ANY;
    serv_addr -> sin_family = AF_INET;
}


static void bind_socket_(int sockfd, struct sockaddr_in* serv_addr) {
    // привязать адрес хоста используя bind()
    if (bind(sockfd, (struct sockaddr *) serv_addr, sizeof(&serv_addr)) < 0) {
        perror("ERROR on binding\n");
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(1);
    }
}


static void make_socket_nonblocking_(int sockfd) {
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0 ) | O_NONBLOCK ) < 0) {
        printf("error in fcntl errno=%i\n", errno);
    }
}


void* accepting_thread(void* arg) {
    struct sockaddr_in serv_addr, cli_addr;
    int initial_socket, newsockfd;
    unsigned int clilen;


    // открытие начального сокета
    initial_socket = ((Accepting_thread_input*)arg)->initial_sockfd;

    if (initial_socket < 0) {
        fprintf(stderr, "ERROR opening init socket\n");
        exit(1);
    }

    make_socket_reusable_(initial_socket);

    // инициализация структуры initial_socket
    fill_server_info_(&serv_addr, ((Accepting_thread_input*) arg) -> port_number);

    // привязать адрес хоста используя bind()
    bind_socket_(initial_socket, &serv_addr);

    make_socket_nonblocking_(initial_socket);

    listen(initial_socket, 5);
    clilen = sizeof(cli_addr);

    while(1) {
        newsockfd = accept(initial_socket, (struct sockaddr *) &cli_addr, &clilen);

        if (errno != EWOULDBLOCK) {
            free(arg);
            fprintf(stdout,"Accepting thread: exit.\n");
            pthread_exit(0);
        }

        if (newsockfd > 0) {
            create_listening_thread(newsockfd);
            list_of_clients_add(list_of_clients_make_new_item(newsockfd));
        }
    }
}


Accepting_thread_input* init_accepting_thread_input_structure(int port_number, int initial_sockfd) {
    Accepting_thread_input* new_input_structure = (Accepting_thread_input*) malloc(sizeof(Accepting_thread_input));

    new_input_structure -> port_number = port_number;
    new_input_structure -> initial_sockfd = initial_sockfd;

    return new_input_structure;
}


pthread_t create_accepting_thread(int port_number, int initial_sockfd) {
    pthread_t new_thread;
    Accepting_thread_input* accepting_thread_input = init_accepting_thread_input_structure(port_number, initial_sockfd);

    if( pthread_create(&new_thread, NULL, accepting_thread, (void*) accepting_thread_input) ) {
        return -1;
    }

    return new_thread;
}
