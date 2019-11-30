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
void sendFile();
void receiveFile();
void enterFileName();

int sockfd;
char fileName[50];

int main(int argc, char *argv[]) {
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[MAXSIZE];
    char option[3];
    int optionInt;

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
    signal(SIGINT, sigHandlerOut);

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
    while (1) {
        printf("Выберите опцию:\n1 - Скачать файл\n2 - Загрузить файл\n");
        fgets(option, 3,stdin);
        optionInt = atoi(option);
        if (optionInt == 1) {
            enterFileName();
            receiveFile();
        } else if (optionInt == 2) {
            enterFileName();
            sendFile();
        } else {
            printf("Неверная опция\n");
        }
    }
    sendto(sockfd, (const char *) buffer, strlen(buffer),
           MSG_CONFIRM, (const struct sockaddr *) &serv_addr,
           sizeof(serv_addr));
}

void enterFileName(){
    memset(&fileName, 0, sizeof(fileName));
    printf("Введите имя файла\n");
    fgets(fileName,49,stdin);
}

void sendFile(){
    
}

void receiveFile(){

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




