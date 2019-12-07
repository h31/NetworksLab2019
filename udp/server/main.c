#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <stdint.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define MAX_SIZE 516
#define DATA_SIZE 512
#define READ_OPCODE 1
#define WRITE_OPCODE 2
#define DATA_OPCODE 3
#define ACK_OPCODE 4
#define ERROR_OPCODE 5


typedef struct {
    struct sockaddr_in addr;
    FILE *f;
    uint16_t blockNumber;
} clientInfo;

void sigHandler(int sig);

void receivePacket();

void checkOpcode();

void readingDataFromFile(int i);

void firstRequestFromClient();

struct sockaddr_in serv_addr;
clientInfo clients[MAX_CLIENTS];
int sockfd;
char packet[MAX_SIZE];
char response[MAX_SIZE];
int receivedLength, packetLength;
struct sockaddr_in cli_addr;
uint16_t opCode, blockNumber, errorCode;
socklen_t len = sizeof(serv_addr);
char dataOut[DATA_SIZE];
FILE *file;
char *fileName;
int readedSymbols;
char error_1[] = "Такого файла не существует";
char error_2[] = "Файл с таким именем уже существует";

int main(int argc, char *argv[]) {
    //инициализация
    uint16_t port;
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
    int m = sendto(sockfd, (const char *) packet, packetLength,
                   MSG_CONFIRM, (const struct sockaddr *) &cli_addr,
                   sizeof(cli_addr));
    if (m < 0) {
        printf("Невозможна передача данных клиенту\n");
        perror("ERROR");
        exit(1);
    }
}

void formAndSendFirstResponse() {
    bzero(packet, MAX_SIZE);
    memcpy(packet, &opCode, 2);
    memcpy(packet + 2, &blockNumber, 2);
    memcpy(packet + 4, dataOut, DATA_SIZE);
    sendPacket();
}


void receivePacket() {
    receivedLength = recvfrom(sockfd, (char *) response, MAX_SIZE,
                              MSG_WAITALL, (struct sockaddr *) &cli_addr,
                              &len);
    opCode = ntohs(*(uint16_t *) response);
    checkOpcode();
}

void checkOpcode() {
    if (opCode == READ_OPCODE || opCode == WRITE_OPCODE) {
        firstRequestFromClient();
    } else {
        //ищем нашего клиента в массиве
        int i = 0;
        while (clients[i].addr.sin_addr.s_addr != cli_addr.sin_addr.s_addr && i < MAX_CLIENTS) {
            i++;
        }
        //проыеряем,что нашли его, а не вышли за границы
        if (i < 100) {
            if (opCode == DATA_OPCODE) {
                blockNumber = ntohs(*(uint16_t *) (response + 2));
                fwrite((response + 4), sizeof(char), receivedLength - 4, clients[i].f);
                opCode = htons(ACK_OPCODE);
                blockNumber = htons(blockNumber);
                formAndSendFirstResponse();
                if (receivedLength < MAX_SIZE) {
                    fclose(clients[i].f);
                    bzero(&clients[i], sizeof(clients[i]));
                }
            } else if (opCode == ACK_OPCODE) {
                if (clients[i].blockNumber < 0) {
                    fclose(clients[i].f);
                    bzero(&clients[i], sizeof(clients[i]));
                } else {
                    readingDataFromFile(i);
                }
            } else {
                printf("Что-то не так\n");
                exit(1);
            }
        } else {
            printf("Неизвестный клиент\n");
        }
    }
}

void firstRequestFromClient() {
    //ищем свободное месечко для клиента
    int i = 0;
    while (clients[i].addr.sin_addr.s_addr != 0 && i < MAX_CLIENTS) {
        i++;
    }
    if (i < 100) {
        //записываем адрес клиента в массив клиентов
        clients[i].addr = cli_addr;
        if (opCode == READ_OPCODE) { //чтение из файла
            fileName = response + 2;
            file = fopen(fileName, "r");
            if (file == NULL) {
                //Файл не найден
                opCode = htons(ERROR_OPCODE);
                errorCode = htons(1);
                packetLength = 4 + strlen(error_1) + 1;
                memcpy(packet, &opCode, 2);
                memcpy(packet + 2, &errorCode, 2);
                memcpy(packet + 4, error_1, strlen(error_1) + 1);
                sendPacket();
            } else {
                clients[i].f = file;
                clients[i].blockNumber = 0;
                readingDataFromFile(i);
            }
        } else { //запись в файл
            fileName = response + 2;
            //проверяю не существует ли такой файл
            if ((file = fopen(fileName, "r")) == NULL) {
                file = fopen(fileName, "w");
                if (file == NULL) {
                    printf("Невозможно создать файл\n");
                } else {
                    clients[i].f = file;
                    clients[i].blockNumber = 0;
                    opCode = htons(ACK_OPCODE);
                    blockNumber = htons(0);
                    packetLength = 4;
                    memcpy(packet, &opCode, 2);
                    memcpy(packet + 2, &blockNumber, 2);
                    sendPacket();
                }
            } else {
                fclose(file);
                opCode = htons(ERROR_OPCODE);
                errorCode = htons(6);
                packetLength = 4 + strlen(error_2) + 1;
                memcpy(packet, &opCode, 2);
                memcpy(packet + 2, &errorCode, 2);
                memcpy(packet + 4, error_2, strlen(error_2) + 1);
                sendPacket();
            }
        }
    } else {
        printf("Невозможно подключить клиента\n");
    }
}

void readingDataFromFile(int i) {
    bzero(dataOut, DATA_SIZE);
    opCode = htons(DATA_OPCODE);
    clients[i].blockNumber = clients[i].blockNumber + 1;
    blockNumber = htons(clients[i].blockNumber);
    readedSymbols = fread(dataOut, sizeof(char), DATA_SIZE, clients[i].f);
    packetLength = 4 + readedSymbols;
    formAndSendFirstResponse();
    if (readedSymbols < DATA_SIZE) {
        clients[i].blockNumber = -1;
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




