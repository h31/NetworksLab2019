#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "../../common.h"
#include "pthread.h"

char *user_name = NULL;

void *client_read(int sockfd) {
    while (1) {
        char *buffer_message = calloc(BUFF_SIZE, sizeof(char));
        if (read(sockfd, buffer_message, BUFF_SIZE - 1) < 0) {
            PERROR_AND_EXIT("ERROR reading from socket");
        }
        printf("\n%s\n%s:", buffer_message, user_name);

    }

}

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    ///////////////////////////////////////////////////////
    ///////////////// parsing arguments ///////////////////
    ///////////////////////////////////////////////////////
    if (argc < 3) {
        fprintf(stderr, "Illegal amount of arguments < 2.\n"
                        "-s(server) and -p(port) must be passed\n");
        exit(1);
    }

    int opt;
    while ((opt = getopt(argc, argv, "s:p:n:")) != -1) {
        switch (opt) {
            case 's':
                server = gethostbyname(optarg);
                if (server == NULL) {
                    fprintf(stderr, "ERROR, no such host\n");
                    exit(0);
                }
                break;
            case 'p':
                portno = (uint16_t) atoi(optarg);
                break;
            case 'n':
                user_name = strdup(optarg);
                break;
            default:
                /* unrecognised opt ... add your error condition */
                printf("Unrecognized opt = %s", optarg);
                exit(1);
        }
    }

    if (user_name == NULL) {
        PERROR_AND_EXIT("user_name = NULL\n")
    }
    printf("user_name = %s\n", user_name);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        PERROR_AND_EXIT("ERROR opening socket");
    }

    /* Now connect to the server */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
        < 0) {
        PERROR_AND_EXIT("ERROR connecting");
    }

    pthread_t thread_read;

    pthread_create(&thread_read, NULL, (void *(*)(void *)) client_read, sockfd);

    if (write(sockfd, user_name, strlen(user_name)) < 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }

    printf("\n%s:", user_name);
    while (1) {
        char *buffer_message = calloc(BUFF_SIZE, sizeof(char));
        fgets(buffer_message, BUFF_SIZE - 1, stdin);
        if (write(sockfd, buffer_message, strlen(buffer_message)) < 0) {
            PERROR_AND_EXIT("ERROR writing to socket");
        }
    }

    return 0;
}
