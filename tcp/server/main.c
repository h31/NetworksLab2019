#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <signal.h>
#include "../common.h"
#include "pthread.h"

#define HEADER_SIZE         4
#define MAX_MESSAGE_SIZE    256

Client *first_client;
int global_sockfd;
pthread_mutex_t mutex;

/*------------------- SERVER DELETE CLIENT ---------------------------------*/
void server_delete_client(Client *client) {
    printf("Delete client: name = %s, sockfd = %d\n", client->name, client->sockfd);
    close(client->sockfd);
    if (client->prev_client != NULL) {
        printf("prev = %s, ", client->prev_client->name);
        client->prev_client->next_client = client->next_client;
    }
    if (client->next_client != NULL) {
        printf("next = %s", client->next_client->name);
        client->next_client->prev_client = client->prev_client;
    }
    printf("\n");
    pthread_t client_thread = client->thread;
    free(client);
    pthread_cancel(client_thread);

}

/*------------------- SERVER GET MESSAGE ---------------------------------*/
Message serv_get_message(Client *client) {
    Message message;

    message.size = 0;

    if (read(client->sockfd, &message.size, HEADER_SIZE) <= 0) {
        server_delete_client(client);
    }

    message.buffer = calloc(message.size, sizeof(char));

    if (readN(client->sockfd, message.buffer, message.size) <= 0) {
        server_delete_client(client);
    }
    return message;
}

/*------------------- SERVER SEND MESSAGE ---------------------------------*/
void serv_send_response(Client *client, Message message) {

    if (write(client->sockfd, &message.size, HEADER_SIZE) <= 0) {
        server_delete_client(client);
    }

    if (write(client->sockfd, message.buffer, message.size) <= 0) {
        server_delete_client(client);
    }
}

/*------------------- SERVER PROCESS CLIENT -------------------------------------*/
void serv_process_client(Client *client) {

    client->status = CL_STAT_ON;

    for (;;) {
        Message message = serv_get_message(client);
        printf("\nRECEIVED:%s:message = %s(size = %d)\n", client->name, message.buffer, message.size);
        message.buffer = str_concat(str_concat(client->name, ":"), message.buffer);
        message = *get_new_message(message.buffer);

        VERBOSE_MUTEX_LOCK(&mutex, "sending messages to all clients");

        Client *another_client = first_client->next_client;
        while (another_client->next_client != NULL) {

            if (another_client != client &&
                another_client->status == CL_STAT_ON) {

                serv_send_response(another_client, message);
                printf("SENDED:message = %s(size = %d) to %s\n",
                       message.buffer,
                       message.size,
                       another_client->name);

            }

            another_client = another_client->next_client;
        }

        VERBOSE_MUTEX_UNLOCK(&mutex, "sending messages to all clients");

    }

}

/*------------------- SERVER SIGINT HANDLER --------------------------------------------*/
void server_sigint_handler(int signo) {
    printf("Closing server...\n");
    Client *deleted_client = first_client;
    if (deleted_client != NULL) {
        while (deleted_client->next_client != NULL) {
            deleted_client = deleted_client->next_client;
            server_delete_client(deleted_client->prev_client);
        }
    }
    server_delete_client(deleted_client);

    printf("Close socket = %d\n", global_sockfd);
    close(global_sockfd);
    printf("Destroy mutex\n");
    pthread_mutex_destroy(&mutex);
    printf("Server closed...\n");
    exit(0);
}

/*------------------- GET LAST NOT NULL CLIENT ----------------------------------------------*/
Client *get_last_not_null_client() {
    Client *free_client = first_client;
    printf("\n-------------------------------\n");
    printf("Clients in list:\n");
    int i = 1;
    while (free_client->next_client != NULL) {
        __uint32_t cli_addr = free_client->next_client->sockaddr->sin_addr.s_addr;
        printf("%d:(%d.%d.%d.%d)\t:%s\t:%d\n", i,
               (u_char) cli_addr, (u_char) (cli_addr >> 8), (u_char) (cli_addr >> 16), (u_char) (cli_addr >> 24),
               free_client->next_client->name,
               free_client->next_client->sockfd);
        free_client = free_client->next_client;
        i++;
    }

    return free_client;

}

/*------------------- MAIN -----------------------------------------------------*/
int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;

    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                portno = (uint16_t) atoi(optarg);
                break;
            default:
                /* unrecognised opt ... add your error condition */
                printf("Unrecognized opt = %s", optarg);
                break;
        }
    }
    if (signal(SIGINT, server_sigint_handler) == SIG_ERR) {
        PERROR_AND_EXIT("client_sigint_handler initialized failed");
    }

    /* First call to socket() function */
    global_sockfd = sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        PERROR_AND_EXIT("ERROR opening socket");
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) {
        PERROR_AND_EXIT("setsockopt(SO_REUSEADDR) failed");
    }

    printf("Portno = %d\n", portno);
    printf("Sockfd = %d\n", sockfd);
    printf("IP ADDRESS:\n");
    system(" ifconfig | sed -En 's/127.0.0.1//;s/.*inet (addr:)?(([0-9]*\\.){3}[0-9]*).*/\\2/p'");

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        PERROR_AND_EXIT("ERROR on binding");
    }

    /* Now start listening for the first_client, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        PERROR_AND_EXIT("ERROR creating socket")
    }

    first_client = get_new_client_empty();
    first_client->name = "first client";
    for (;;) {
        /* Accept actual connection from the client */
        Client *free_client = get_last_not_null_client();
        Client *new_client = get_new_client_empty();
        free_client->next_client = new_client;
        new_client->prev_client = free_client;

        new_client->sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (new_client->sockfd <= 0) {
            PERROR_AND_EXIT("ERROR on accept");
        }

        Message name_mess = serv_get_message(new_client);
        new_client->name = name_mess.buffer;
        new_client->sockaddr = &cli_addr;
        printf("New client accepted: name = %s, sockfd = %d\n", new_client->name,
               new_client->sockfd);
        pthread_create(&new_client->thread, NULL, (void *) serv_process_client,
                       new_client);//TODO incorrect, but not critical passing

    }

    return 0;
}
