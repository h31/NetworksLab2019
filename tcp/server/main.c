#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include "../common.h"

#define MAX_CLIENT_NUM      4
#define HEADER_SIZE         4
#define MAX_MESSAGE_SIZE    256
#define CL_STATUS_DISC      0
#define CL_STATUS_CONN      1
#define NO_PLACES_INDEX     0
#define NO_PLACES_STRING    "Sorry, no places"

#include "pthread.h"


Client clients[MAX_CLIENT_NUM];

/*------------------- GETTERS --------------------------------------------*/
int get_free_index() {
    for (int i = 1; i < MAX_CLIENT_NUM; ++i) {
        clients[i].status = CL_STATUS_DISC;
        return i;
    }
    return NO_PLACES_INDEX;
}

/*------------------- SERVER GET MESSAGE ---------------------------------*/
Message serv_get_message(int sockfd) {
    Message message;

    message.buffer = malloc(MAX_MESSAGE_SIZE * sizeof(char));
    bzero(message.buffer, MAX_MESSAGE_SIZE);
    message.size = 0;

    if (read(sockfd, &message.size, sizeof(int)) <= 0) {
        PERROR_AND_EXIT("ERROR reading message size");
    }
    if (read(sockfd, message.buffer, message.size) <= 0) {
        PERROR_AND_EXIT("ERROR reading message")
    }
    return message;
}

/*------------------- SERVER SEND MESSAGE ---------------------------------*/
void serv_send_response(int sockfd, Message message) {

    if (write(sockfd, &message.size, HEADER_SIZE) <= 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }

    if (write(sockfd, message.buffer, message.size) <= 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }
}

void serv_send_no_cap_message(int sockfd) {
    size_t message_size = (size_t) strdup(NO_PLACES_STRING);

    if (write(sockfd, &message_size, HEADER_SIZE) <= 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }

    if (write(sockfd, NO_PLACES_STRING, message_size) <= 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }
}

/*------------------- SERVER SEND MESSAGE ---------------------------------*/
void serv_process_client(int i) {
    for (;;) {
        Message message = serv_get_message(clients[i].sockfd);
        printf("RECEIVED:%s:message = %s(size = %d)\n", clients[i].name, message.buffer, message.size);
        serv_send_response(clients[i].sockfd, message);
        printf("SENDED:%s:message = %s(size = %d)\n", clients[i].name, message.buffer, message.size);

    }

}


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

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        PERROR_AND_EXIT("ERROR opening socket");
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

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    for (;;) {
        /* Accept actual connection from the client */

        int i = get_free_index();
        clients[i].sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (clients[i].sockfd <= 0) {
            PERROR_AND_EXIT("ERROR on accept");
        }

        Message name_mess = serv_get_message(clients[i].sockfd);
        clients[i].name = name_mess.buffer;

        if (i == NO_PLACES_INDEX) {
            serv_send_no_cap_message(clients[i].sockfd);
        } else {
            clients[i].status = CL_STATUS_CONN;
            printf("New client accepted: name = %s, i = %d, sockfd = %d\n", clients[i].name, i, clients[i].sockfd);
            pthread_create(&clients[i].thread, NULL, serv_process_client, i);
        }
    }

    return 0;
}
