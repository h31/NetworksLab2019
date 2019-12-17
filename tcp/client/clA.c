#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>

#include <string.h>
#include "clList.h"

char name[NAME_L];
char buffer[BUF_SIZE];

void *console_handler(void *args);

void message_handler(int socket);

int main(int argc, char *argv[]) {
    int sockfd, n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;


    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }


    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Your name: ");
    bzero(name, NAME_L);
    fgets(name, NAME_L, stdin);
    check_line(name, NAME_L, '\n', '\0');

    /* Send name to the server */
    uint32_t len = strlen(name) + 1;
    n = write(sockfd, (char *) &len, sizeof(len));
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    n = write(sockfd, name, strlen(name) + 1);
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    pthread_t thrd;
    pthread_create(&thrd, NULL, console_handler, &sockfd);

    while (1) {
        /* Now read server response */
        message_handler(sockfd);
    }

    close(sockfd);
    return 0;
}

void *console_handler(void *args) {
    int n;
    int fd = *(int *) args;
    while (1) {
        bzero(buffer, BUF_SIZE);
        fgets(buffer, BUF_SIZE, stdin);
        check_line(buffer, BUF_SIZE, '\n', '\0');
        /* Send message to the server */
        uint32_t len = strlen(buffer) + 1;
        n = write(fd, (char *) &len, sizeof(len));
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
        n = write(fd, &buffer, len);
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
    }
}

//Snader Effective TCP programming listing 2.13
void message_handler(int socket) {
    size_t reclen = 0;
    ssize_t rc;
    rc = readn(socket, (char *) &reclen, LEN_MESSAGE);
    if (rc <= 0) {
        perror("ERROR reading from socket \n");
        exit(1);

    } else {
        char *text = malloc(sizeof(char) * reclen);
        readn(socket, text, reclen);
        printf(" New message %s\n", text);
        free(text);
    }
}

