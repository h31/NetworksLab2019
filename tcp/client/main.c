#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>
#include "../header.h"
#include "../params.h"

void send_msg(void *arg);

void get_msg(void *arg);

int flag;

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char name[NAME_LEN];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

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

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    printf("Enter your name:");
    bzero(name, NAME_LEN);
    fgets(name, NAME_LEN, stdin);
    if (write(sockfd, name, NAME_LEN) < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    printf("Write /exit to exit chat\n");

    pthread_t tid_send;
    if (pthread_create(&tid_send, NULL, (void *) send_msg, &sockfd) != 0) {
        printf("thread has not created\n");
        exit(1);
    }

    pthread_t tid_get;
    if (pthread_create(&tid_get, NULL, (void *) get_msg, &sockfd) != 0) {
        printf("thread has not created\n");
    }

    flag = 0;

    pthread_join(tid_send, NULL);
    pthread_join(tid_get, NULL);
    return 0;
}

void send_msg(void *arg) {
    int sock = *(int *) arg;
    char buffer[MSG_LEN];

    while (1) {
        bzero(buffer, MSG_LEN);
        fgets(buffer, MSG_LEN, stdin);

        if (write(sock, buffer, MSG_LEN) < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }

        make_str(buffer);
        if (strcmp(buffer, "/exit") == 0) {
            printf("Goodbye\n");
            close(sock);
            exit(EXIT_SUCCESS);
        }
    }
}

void get_msg(void *arg) {
    int sock = *(int *) arg;
    char buffer[SEND_MSG_LEN];
    while (1) {
        bzero(buffer, SEND_MSG_LEN);

        if (read(sock, buffer, SEND_MSG_LEN) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }

        printf("%s\n", buffer);
    }
}