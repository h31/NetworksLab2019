#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include<time.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>

#include <termios.h>

void sigHandlerOut(int sig);

void closeClient();

void getListOfProducts();

int readN(int socket, void *buf, int length);

char *readMessage();

void printListOfProducts(char const *answer);

void enteringDataForRequest(_Bool);

void addProduct();

void buyProduct();

int sockfd;
#define PROD_NAME_SIZE 50

#define ERROR_OPCODE 1
#define LIST_OF_PROD_OPCODE 2
#define ADD_PROD_OPCODE 3
#define BUY_PROD_OPCODE 4
#define ACK_OPCODE 5
#define GET_LIST_OF_PROD_OPCODE 6

uint16_t opcode;
int packet_length;
int message_size;

char prodName[PROD_NAME_SIZE];
int cost;
int count;
char costChar[5];
char countChar[5];

int main(int argc, char *argv[]) {
    //объявление переменных
    char option[3];
    int optionInt;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //проверка, что все аргументы введены
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    //номер порта
    portno = (uint16_t) atoi(argv[2]);

    //создание сокета и проверка
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    //обработчик закрытия клиента
    signal(SIGINT, sigHandlerOut);

    //нахожу мой сервер и проверяю
    server = gethostbyname(argv[1]);
    //server = argv[1];
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    //соединение с сервером
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }


    while (1) {
        printf("Выберите опцию:\n"
               "1 - Показать список товаров \n"
               "2 - Добавить товар \n"
               "3 - Купить товар \n"
               "4 - Завершить работу\n");
        fgets(option, 3, stdin);
        optionInt = atoi(option);
        if (optionInt == 1) {
            getListOfProducts();
        } else if (optionInt == 2) {
            addProduct();
        } else if (optionInt == 3) {
            buyProduct();
        } else if (optionInt == 4) {
            closeClient();
            break;
        } else {
            printf("Неверная опция\n");
        }
    }
}

void getListOfProducts() {
    char *answer;
    packet_length = 6;
    char packet[packet_length];
    opcode = GET_LIST_OF_PROD_OPCODE;
    memcpy(packet, &packet_length, 4);
    memcpy(packet + 4, &opcode, 2);
    if (write(sockfd, packet, packet_length) <= 0) {
        closeClient();
    }
    answer = readMessage();
    printListOfProducts(answer);
    free(answer);
}

void buyProduct() {
    enteringDataForRequest(false);
    packet_length = 11 + strlen(prodName);
    char packet[packet_length];
    opcode = BUY_PROD_OPCODE;
    memcpy(packet, &packet_length, 4);
    memcpy(packet + 4, &opcode, 2);
    memcpy(packet + 6, &count, 4);
    memcpy(packet + 10, prodName, strlen(prodName) + 1);
    if (write(sockfd, packet, packet_length) <= 0) {
        closeClient();
    }
    char *buffer;
    buffer = readMessage();
    if (*(uint16_t *) buffer == ACK_OPCODE && *(uint16_t*) (buffer + 2) == 1) {
        printf("Товар куплен\n");
    } else if (*(uint16_t *) buffer == ERROR_OPCODE) {
        int pointer = 2;
        printf("Ошибка: ");
        while (*(buffer + pointer) != '\0') {
            printf("%c", *(buffer + pointer));
            pointer++;
        }
        printf("\n");
    } else {
        printf("Произошла ошибка при покупке товара\n");
    }
}

void addProduct() {
    enteringDataForRequest(true);
    packet_length = 15 + strlen(prodName);
    char packet[packet_length];
    opcode = ADD_PROD_OPCODE;
    memcpy(packet, &packet_length, 4);
    memcpy(packet + 4, &opcode, 2);
    memcpy(packet + 6, &count, 4);
    memcpy(packet + 10, &cost, 4);
    memcpy(packet + 14, prodName, strlen(prodName) + 1);
    if (write(sockfd, packet, packet_length) <= 0) {
        closeClient();
    }
    char *buffer;
    buffer = readMessage();
    if (*(uint16_t *) buffer == ACK_OPCODE && *(uint16_t*) (buffer + 2) == 2) {
        printf("Товар добавлен\n");
    } else if (*(uint16_t *) buffer == ERROR_OPCODE) {
        int pointer = 2;
        printf("Ошибка: ");
        while (*(buffer + pointer) != '\0') {
            printf("%c", *(buffer + pointer));
            pointer++;
        }
        printf("\n");
    } else {
        printf("Произошла ошибка при добавлении товара\n");
    }
}

void enteringDataForRequest(_Bool add) {
    //TODO обработка неправильного ввода (strtol)
    bzero(&prodName, sizeof(prodName));
    bzero(&costChar, sizeof(costChar));
    bzero(&countChar, sizeof(countChar));
    printf("Введите название товара\n");
    fgets(prodName, PROD_NAME_SIZE - 1, stdin);
    prodName[strlen(prodName) - 1] = '\0';
    if (add) {
        printf("Введите цену товара\n");
        fgets(costChar, 5, stdin);
        cost = atoi(costChar);
    }
    printf("Введите количество товара\n");
    fgets(countChar, 5, stdin);
    count = atoi(countChar);
}

void printListOfProducts(char const *answer) {
    int cost;
    int count;
    int pointer = 2;
    opcode = *(uint16_t *) answer;
    if (opcode == LIST_OF_PROD_OPCODE) {
        if (pointer < message_size) {
            printf("Цена товара   |   Количество товара  |  Название товара\n");
            while (pointer < message_size) {
                cost = *(int *) (answer + pointer);
                count = *(int *) (answer + pointer + 4);
                printf("     %d             %d              ", cost, count);
                pointer += 8;
                while (*(answer + pointer) != '\0') {
                    printf("%c", *(answer + pointer));
                    pointer++;
                }
                printf("\n");
                pointer++;
            }
        } else {
            printf("В магазине пока нет товаров...\n");
        }
    } else {
        printf("Ошибка\n");
    }
}

char *readMessage() {
    //считываю длину сообщения - 1 байт
    if (readN(sockfd, &message_size, sizeof(int)) <= 0) {
        perror("ERROR reading from socket");
        closeClient();
    }
    message_size -= 4;
    char *buffer = (char *) malloc(message_size);
    //считываю остальное сообщение
    if (readN(sockfd, buffer, message_size) <= 0) {
        perror("ERROR reading from socket");
        closeClient();
    }
    return buffer;
}

int readN(int socket, void *buf, int length) {
    int result = 0;
    int readedBytes = 0;
    int messageLength = length;
    while (messageLength > 0) {
        readedBytes = read(socket, buf + result, messageLength);
        if (readedBytes <= 0) {
            return -1;
        }
        result += readedBytes;
        messageLength -= readedBytes;
    }
    return result;
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
    printf("\nЗавершение программы\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(1);
}


