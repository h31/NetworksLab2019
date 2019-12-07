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
#define DATA_SIZE 512
#define READ_OPCODE 1
#define WRITE_OPCODE 2
#define DATA_OPCODE 3
#define ACK_OPCODE 4
#define ERROR_OPCODE 5

void sigHandlerOut(int sig);

void closeClient();

void sendFile();

void receiveFile();

void enterFileName();

void formRequest();

void sendPacket();

void getResponse();

void openFileForWrite();

void openFileForRead();

int sockfd;
char fileName[FILE_SIZE];
uint16_t opcode, blockNumber;
char packet[MAX_SIZE];
char response[MAX_SIZE];
char *data;
char dataOut[DATA_SIZE];
struct sockaddr_in serv_addr;
static const char MODE[] = "octet";
int packetLength = MAX_SIZE;
int readedSymbols = DATA_SIZE;
socklen_t len = sizeof(serv_addr);
int lengthToSend = 0;
FILE *file;

int main(int argc, char *argv[]) {
    uint16_t portno;
    struct hostent *server;
    char buffer[MAX_SIZE];
    char option[3];
    int optionInt;

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
        enterFileName();
        if (optionInt == 1) {
            openFileForWrite();
            receiveFile();
        } else if (optionInt == 2) {
            openFileForRead();
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
    fileName[strlen(fileName) - 1] = '\0';
}

void formRequest() {
    lengthToSend = 4 + strlen(fileName)+1+strlen(MODE)+1;
    bzero(packet, MAX_SIZE);
    memcpy(packet, &opcode, 2);
    memcpy(packet + 2, fileName, strlen(fileName) + 1);
    memcpy(packet + 2 + strlen(fileName) + 1, MODE, strlen(MODE) + 1);
}

void sendPacket() {
    sendto(sockfd, (const char *) packet, lengthToSend,
           MSG_CONFIRM, (const struct sockaddr *) &serv_addr,
           sizeof(serv_addr));
}

void getResponse() {
    bzero(response, MAX_SIZE);
    packetLength = recvfrom(sockfd, (char *) response, MAX_SIZE,
                            MSG_WAITALL, (struct sockaddr *) &serv_addr,
                            &len);
    opcode = ntohs(*(uint16_t *) response);
    blockNumber = ntohs(*(uint16_t *) (response + 2));
    data = response + 4;
}

void sendFile() {
    opcode = htons(WRITE_OPCODE);
    formRequest();
    sendPacket();
    while (1) {
        getResponse();
        if (readedSymbols != DATA_SIZE) break;
        if (opcode == ACK_OPCODE) {
            //отправила данные прочитала ответ
            bzero(dataOut, DATA_SIZE);
            readedSymbols = fread(dataOut, sizeof(char), DATA_SIZE, file);
            lengthToSend = readedSymbols+4;
            opcode = htons(DATA_OPCODE);
            blockNumber = htons(blockNumber + 1);
            memcpy(packet, &opcode, 2);
            memcpy(packet + 2, &blockNumber, 2);
            memcpy(packet + 4, dataOut, readedSymbols);
            sendPacket();
        } else if (opcode == ERROR_OPCODE) {
            printf("Error: %s\n", data);
            break;
        } else {
            printf("Ошибка, запись невозможна\n");
            exit(1);
        }
    }
    readedSymbols = DATA_SIZE;
    fclose(file);
    if (opcode != ERROR_OPCODE) {
        printf("Файл передан\n");
    }
}

void receiveFile() { //сервер мне присылает, я у себя записываю
    opcode = htons(READ_OPCODE);
    lengthToSend = MAX_SIZE;
    formRequest();
    sendPacket();
    while (1) {
        getResponse();
        if (opcode == DATA_OPCODE) {
            fwrite(data, sizeof(char), packetLength - 4, file);
            bzero(packet, MAX_SIZE);
            opcode = htons(ACK_OPCODE);
            blockNumber = htons(blockNumber);
            memcpy(packet, &opcode, 2);
            memcpy(packet + 2, &blockNumber, 2);
            sendPacket();
        } else if (opcode == ERROR_OPCODE) {
            printf("Error: %s\n", data);
            break;
        } else {
            printf("Ошибка, чтение невозможно\n");
            exit(1);
        }
        if (packetLength != MAX_SIZE) break;
    }
    fclose(file);
    if (opcode != ERROR_OPCODE) {
        printf("Файл получен\n");
    }
}

void openFileForWrite() {
    file = fopen(fileName, "w");
    if (file == NULL) {
        printf("Невозможно записать в файл\n");
        exit(1);
    }
}

void openFileForRead() {
    file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Файл с таким названием не найден\n");
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




