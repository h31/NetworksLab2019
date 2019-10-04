#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "../../common.h"

#include "pthread.h"


int socket_list[MAX_CLIENT_NUM];
char *client_names[MAX_CLIENT_NUM];
pthread_t thread_poll[MAX_CLIENT_NUM];
int client_num = 0;

char *get_name_from_socket(int socket) {
    for (int i = 0; i < client_num; ++i) {
        if (socket_list[i] == socket)
            return client_names[i];
    }
    return NULL;
}


void *serv_client_processing(int newsockfd) {
    while (1) {
        char *buffer = calloc(BUFF_SIZE, sizeof(char));
        if (newsockfd < 0) {
            PERROR_AND_EXIT("ERROR on accept");
        }

        if (read(newsockfd, buffer, BUFF_SIZE - 1) < 0) {
            PERROR_AND_EXIT("ERROR reading from socket");
        }
        buffer = string_concat(string_concat(get_name_from_socket(newsockfd), ":"), buffer);

        printf("\n%s", buffer);

        for (int i = 0; i < client_num; ++i) {
            if (newsockfd != socket_list[i]) {
                printf("Write to socket = %d\n", socket_list[i]);

                if (write(socket_list[i], buffer, strlen(buffer)) < 0) {
                    PERROR_AND_EXIT("ERROR writing to socket");
                }
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int sockfd;
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
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    for (int i = 0; i < MAX_CLIENT_NUM; ++i)
        client_names[i] = calloc(MAX_CLIENT_NAME_SIZE, sizeof(char));

    while (1) {
        /* Accept actual connection from the client */
        if (client_num < MAX_CLIENT_NUM) {
            socket_list[client_num] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            if (read(socket_list[client_num], client_names[client_num], BUFF_SIZE - 1) < 0) {
                PERROR_AND_EXIT("ERROR reading from socket");
            }
            printf("New client accepted: sockfd = %d,name = %s\n", socket_list[client_num], client_names[client_num]);

            pthread_create(&thread_poll[client_num], NULL, serv_client_processing, socket_list[client_num]);
            client_num++;
        }

    }
    return 0;
}
