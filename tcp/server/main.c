#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "../../common.h"

#include "pthread.h"

#define THREAD_NUM 4
#define MAX_MESSAGE_BYTE 18

void *pthr_client_processing(int newsockfd) {
    char buffer[BUFF_SIZE];

    if (newsockfd < 0) {
        PERROR_AND_EXIT("ERROR on accept");
    }

    /* If connection is established then start communicating */
    bzero(buffer, BUFF_SIZE);

    if (read(newsockfd, buffer, BUFF_SIZE - 1) < 0) {
        PERROR_AND_EXIT("ERROR reading from socket");
    }

    printf("Here is the message: %s\n", buffer);

    if (write(newsockfd, "I got your message", MAX_MESSAGE_BYTE) < 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;

    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;

    if (argc < 2) {
        fprintf(stderr, "Illegal amount of arguments < 1.\n"
                        "-p(port) must be passed\n");
        exit(1);
    }

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
    pthread_t thread_poll[THREAD_NUM];
    int thread_i = 0;
    while (1) {
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);

        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        pthread_create(&thread_poll[thread_i], NULL, pthr_client_processing, newsockfd);
    }
    return 0;
}
