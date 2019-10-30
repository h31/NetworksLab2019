#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>
#include "../header.h"

void send_msg(void *arg);

void get_msg(void *arg);

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *name;
    size_t n;
    uint32_t buff_size;

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

    while (1){
        name = NULL;
        n = 0;
        printf("Enter your name (max 20 symbols):");
        buff_size = getline(&name, &n, stdin);
        if(buff_size <= 21) break;
        else printf("Invalid name, too much symbols\n");
    }

    //send name size
    if (write(sockfd, &buff_size, sizeof(int)) < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    //send name
    if (write(sockfd, name, buff_size) < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    printf("Write /exit to exit chat\n\n");

    pthread_t tid_send;
    if (pthread_create(&tid_send, NULL, (void *) send_msg, &sockfd) != 0) {
        printf("thread has not created\n");
        exit(1);
    }

    pthread_t tid_get;
    if (pthread_create(&tid_get, NULL, (void *) get_msg, &sockfd) != 0) {
        printf("thread has not created\n");
    }

    pthread_join(tid_send, NULL);
    pthread_join(tid_get, NULL);
    return 0;
}

void send_msg(void *arg) {
    int sock = *(int *) arg;
    char *buffer;
    uint32_t buff_size;
    size_t n;

    while (1) {
        buffer = NULL;
        n = 0;
        printf(">");
        //fflush(stdout);
        buff_size = getline(&buffer, &n, stdin);

        //send message size
        if (write(sock, &buff_size, sizeof(int)) < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }

        //send message
        if (write(sock, buffer, buff_size) < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }

        make_str_without_line_break(buffer);
        if (strcmp(buffer, "/exit") == 0) {
            printf("Goodbye\n");
            close(sock);
            exit(EXIT_SUCCESS);
        }
    }
}

void get_msg(void *arg) {
    int sock = *(int *) arg;
    char *buffer;
    uint32_t buff_size;
    while (1) {

        //get size of message
        buff_size = 0;
        if (read(sock, &buff_size, sizeof(int)) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }

        //get message
        buffer = (char *) malloc(buff_size);
        if (readn(sock, buffer, buff_size) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }

        printf("\r%s", buffer);

        printf(">");
        fflush(stdout);
        free(buffer);
    }
}