#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>


#define MAXSIZE 516
void sigHandlerOut(int sig);

void closeClient();

int sockfd;

int main(int argc, char *argv[]) {
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[MAXSIZE];
    //int n, len;

    //проверка, что все аргументы введены
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    //номер порта
    portno = (uint16_t) atoi(argv[2]);

    //создание сокета и проверка
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    //обработчик закрытия клиента
    //signal(SIGINT, sigHandlerOut);

    //нахожу мой сервер и проверяю
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    bzero(buffer, MAXSIZE);
    fgets(buffer,MAXSIZE,stdin);
    sendto(sockfd, (const char *)buffer, strlen(buffer),
           MSG_CONFIRM, (const struct sockaddr *) &serv_addr,
           sizeof(serv_addr));
    printf("отправил %s\n",buffer);
    return 0;
}

//обработчик закрытия клиента
void sigHandlerOut(int sig) {
    if (sig != SIGINT) return;
    else {
        closeClient();
    }
}

//закрытие клиента
void closeClient() {
    printf("\nВыход из чата\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(1);
}




