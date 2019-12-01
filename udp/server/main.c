#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <stdint.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define MAX_SIZE 516

typedef struct {
    struct sockaddr_in addr;
    FILE *f;
    int blockNumber;
} clientInfo;

void sigHandler(int sig);

void receivePacket();

void checkOpcode();

clientInfo clients[MAX_CLIENTS];
int sockfd;
char packet[MAX_SIZE];
char response[MAX_SIZE];
int len, n;
struct sockaddr_in cli_addr;
uint16_t opcode, blockNumber;
char *data;

int main(int argc, char *argv[]) {
    //инициализация
    uint16_t port;
    struct sockaddr_in serv_addr;

    if (argc < 2) {
        printf("Введите номер порта\n");
        exit(1);
    }

    //отлавливаем закрытие сервера
    signal(SIGINT, sigHandler);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&cli_addr, 0, sizeof(cli_addr));

    //инициализация структуры сокета
    port = (uint16_t) atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    //переиспользование адреса
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }
    //привязка сокета к адресу и проверка(+переиспользование адреса для сокета)
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    //обнуление массива для хранения клиентов
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        bzero(&clients[i].addr, sizeof(clients[i].addr));
    }
    printf("Сервер запущен\n");

    while (1) {
        receivePacket();
    }
}

void sendPacket() {
     int m = sendto(sockfd, (const char *) packet, 516,
           MSG_CONFIRM, (const struct sockaddr *) &cli_addr,
           sizeof(cli_addr));
     if(m<0){
         printf("Падаю\n");
         exit(1);
     }
}

void formAndSendFirstResponse() {
    bzero(packet, MAX_SIZE);
    memcpy(packet, &opcode, 2);
    memcpy(packet + 2, &blockNumber, 2);
    sendPacket();
}


void receivePacket() {
    n = recvfrom(sockfd, (char *) response, MAX_SIZE,
                 MSG_WAITALL, (struct sockaddr *) &cli_addr,
                 &len);
    opcode = ntohs(*(uint16_t *) response);
    checkOpcode();
}

void checkOpcode() {
    if (opcode == 1 || opcode == 2) {
        int i = 0;
        while (clients[i].addr.sin_addr.s_addr != 0) {
            i++;
        }
        clients[i].addr = cli_addr;
        if (opcode == 1) {
            opcode = htons(3);
            blockNumber = htons(1);
            printf("Начинаю посылать данные\n");
        } else if (opcode == 2) {
            opcode = htons(4);
            blockNumber = htons(0);
            printf("Отвечаю и разрешаю присылать мне\n");
        }
        formAndSendFirstResponse();
    } else {
        printf("Что-то не так\n");
    }
}



//обработчик закрытия сервера
void sigHandler(int sig) {
    if (sig != SIGINT) return;
    else {
        printf("\nСервер остановлен\n");
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(1);
    }
}




