#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <stdint.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define MAX_SIZE 516

void sigHandler(int sig);

int newsockfd[MAX_CLIENTS];
int sockfd;

int main(int argc, char *argv[]) {
    //инициализация
    uint16_t port;
    struct sockaddr_in serv_addr, cli_addr;
    int len, n;
    char buffer[MAX_SIZE];

    if (argc < 2) {
        printf("Введите номер порта");
        exit(1);
    }


    //создание сокета и проверка
    ;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    //инициализация структуры сокета
    memset(&serv_addr, 0, sizeof(serv_addr));//
    memset(&cli_addr, 0, sizeof(cli_addr));
    port = (uint16_t) atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }
    //привязка сокета к адресу и проверка(+переиспользование адреса для сокета)
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    printf("Сервер запущен\n");
    //обнуление массива для хранения клиентов
    //memset(&newsockfd, 0, sizeof(newsockfd));

    //отлавливаем закрытие сервера
    signal(SIGINT, sigHandler);

    //ждем новых клиентов
    while (1) {
        printf("Жду сообщения\n");
        n = recvfrom(sockfd, (char *) buffer, MAX_SIZE,
                     MSG_WAITALL, (struct sockaddr *) &cli_addr,
                     &len);
        buffer[n]='\0';
        printf("Client: %s \n",buffer);
    }
}

//обработчик закрытия сервера
void sigHandler(int sig) {
    if (sig != SIGINT) return;
    else {
        printf("\nСервер остановлен\n");
        //закрываем главный сокет
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(1);
    }
}




