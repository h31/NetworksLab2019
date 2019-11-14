//
// Created by Malik Hiraev on 07.10.2019.
//

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "../common/common.h"

#define MSG_BUFF_SIZE 512

void *terminal_reader(void *args);

void read_message(int fd);

char terminal_buff[MSG_BUFF_SIZE];

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

    // Читаем имя
    printf("Введите свое имя\n");
    char user_name[USER_NAME_SIZE];
    bzero(user_name, USER_NAME_SIZE);
    fgets(user_name, USER_NAME_SIZE, stdin);
    replace(user_name, USER_NAME_SIZE, '\n', '\0');
    /* Send message to the server */
    n = write(sockfd, user_name, USER_NAME_SIZE);
    if (n <= 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, terminal_reader, &sockfd);

    while (1) {
        /* Now read server response */
        read_message(sockfd);
    }
    return 0;
}

void *terminal_reader(void *args) {
    ssize_t n;
    int sockfd = *(int *) args;
    while (1) {
        bzero(terminal_buff, MSG_BUFF_SIZE);
        fgets(terminal_buff, MSG_BUFF_SIZE, stdin);
        replace(terminal_buff, MSG_BUFF_SIZE, '\n', '\0');
        /* Send message to the server */
        size_t len = strlen(terminal_buff) + 1;
        n = write(sockfd, &len, MSG_SIZE_VAL); // Передаем размер сообщения
        if (n <= 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
        n = write(sockfd, &terminal_buff, len); // Передаем само сообщение
        if (n <= 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
    }
}

void read_message(int fd) {
    ssize_t n;
    size_t msg_size = 0;
    // Читаем размер сообщения
    n = readn(fd, (char *) &msg_size, MSG_SIZE_VAL);
    if (n <= 0) { // Клиент умер/отключился
        perror("ERROR reading from socket");
        exit(1);
    } else {
        // Читаем само сообщение
        char *msg = malloc(sizeof(char) * msg_size);
        //Читаем то кол-во сиволов, которое указано в заголовке сообщения
        readn(fd, msg, msg_size);
        printf("\r%s\n", msg);
        free(msg);
    }
}
