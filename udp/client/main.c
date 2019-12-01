#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>


#define MAX_SIZE 516
#define FILE_SIZE 50
#define DATA_SIZE 512

void sigHandlerOut(int sig);

void closeClient();

void sendFile();

void receiveFile();

void enterFileName();

void formRequest();

void sendPacket();

void getResponse();

int sockfd;
char fileName[FILE_SIZE];
uint16_t opcode, blockNumber;
char packet[MAX_SIZE];
char response[MAX_SIZE];
char *data;
struct sockaddr_in serv_addr;
static const char MODE[] = "netascii";
int n;
socklen_t len;

int main(int argc, char *argv[]) {
    uint16_t portno;
    struct hostent *server;
    char buffer[MAX_SIZE];
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

    bzero(buffer, MAX_SIZE);
    while (1) {
        printf("Выберите опцию:\n1 - Скачать файл\n2 - Загрузить файл\n");
        fgets(option, 3, stdin);
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
}

void enterFileName() {
    memset(&fileName, 0, sizeof(fileName));
    printf("Введите имя файла\n");
    fgets(fileName, FILE_SIZE - 1, stdin);
    fileName[strlen(fileName)-1]='\0';
}

void formRequest() {
    bzero(packet, MAX_SIZE);
    memcpy(packet, &opcode, 2);
    memcpy(packet + 2, fileName, strlen(fileName));
    memcpy(packet + 2 + strlen(fileName), MODE, strlen(MODE));
}

void sendPacket() {
    sendto(sockfd, (const char *) packet, 516,
           MSG_CONFIRM, (const struct sockaddr *) &serv_addr,
           sizeof(serv_addr));
}

void getResponse() {
    bzero(response, MAX_SIZE);
    n = recvfrom(sockfd, (char *) response, MAX_SIZE,
                 MSG_WAITALL, (struct sockaddr *) &serv_addr,
                 &len);
    opcode = ntohs(*(uint16_t *) response);
    blockNumber = ntohs(*(uint16_t *) (response + 2));
    data = response + 4;
}

void sendFile() {
    opcode = htons(2);
    formRequest();
    sendPacket();
    getResponse();
    if (opcode == 4 && blockNumber == 0) {
        printf("всё хорошо, можно начинать отсылать\n");
    } else {
        printf("Ошибка, запись невозможна\n");
        exit(1);
    }
}

void receiveFile() {
    opcode = htons(1);
    formRequest();
    sendPacket();
    getResponse();
    if (opcode == 3 && blockNumber == 1) {
        printf("всё хорошо, можно начинать принимать\n");
    } else {
        printf("Ошибка, чтение невозможно\n");
        exit(1);
    }
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
    printf("\nВыход из программы\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(1);
}




