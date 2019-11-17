#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include<time.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>

#include <termios.h>

void sigHandlerOut(int sig);

void *reading(void *sockfd);

void sendMessage(void *message);

void closeClient();

void addNewMessage(char *message);

void printReceivedMessenges();

char *readMessagee();

int readN(int socket, void *buf, int length);

char *receivedMessenges[1024];
bool toRead = true;
int sockfd;
int nextFreeString = 0;


int main(int argc, char *argv[]) {
    //объявление переменных
    int nameLength = (int) strlen(argv[3]) + 3;
    char name[nameLength];
    char timeToSend[8];
    char buffer[256];
    int len;

    pthread_t tid;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //проверка, что все аргументы введены
    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port nickname\n", argv[0]);
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

    //каждому клиенты отдельный поток чтения
    if (pthread_create(&tid, NULL, reading, (void *) (intptr_t) sockfd) < 0) {
        perror("ERROR on create phread");
        exit(2);
    }

    //подготовила имя клиента для отправки
    snprintf(name, sizeof name, "[%s]", argv[3]);
    while (1) {
        if (toRead) { // true - пока не нажата esc, то есть нельзя писать сообщение
            int character;
            struct termios orig_term_attr;
            struct termios new_term_attr;

            // что-то там для терминала, чтобы считать нажатие клавиши
            tcgetattr(fileno(stdin), &orig_term_attr);
            memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
            new_term_attr.c_lflag &= ~(ECHO | ICANON);
            new_term_attr.c_cc[VTIME] = 0;
            new_term_attr.c_cc[VMIN] = 0;
            tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

            //считываю нажатие кливаши
            character = fgetc(stdin);
            tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

            //если нажата esc
            if (character == 0x1B) {
                //не вывожу сообщения в терминал и могу печатать своё сообщение
                toRead = false;
                printf("\nВведите сообщение:\n");
            }
        } else {
            //обнуляю буфер и считываю новое сообщение
            bzero(buffer, 256);
            fgets(buffer, 255, stdin);

            //чсто-то там для узнавания текущего времени
            time_t my_time;
            struct tm *timeinfo;
            time(&my_time);
            timeinfo = localtime(&my_time);

            //подготовила согласно формату время и длину
            snprintf(timeToSend, sizeof timeToSend, "<%d:%d>", timeinfo->tm_hour, timeinfo->tm_min);

            //длина моего сообщения, включая время и имя отправителя
            len = strlen(name)+strlen(timeToSend)+strlen(buffer);

            //отправляю данные согласно протоколу: длинуб имя, время, сообщение
            if (write(sockfd, &len, sizeof(int)) <= 0) {
                closeClient();
            }
            sendMessage(name);
            sendMessage(timeToSend);
            sendMessage(buffer);

            //вывожу на экран сообщения, которые пришли пока я писала
            printReceivedMessenges();
            //возвращаюсь в режим принятия сообщений, сама снова не могу писать пока не нажму esc
            toRead = true;
        }
    }
}

//поток чтения сообщений с сервера
void *reading(void *sockfd) {
    char *buffer;
    while (1) {
        buffer = readMessagee();
        if (toRead) {
            printf("%s\n", buffer);
        } else {
            addNewMessage(buffer);
        }
        free(buffer);
    }
}

//отправка сообщения
void sendMessage(void *message) {
    if (write(sockfd, message, strlen(message)) <= 0) {
        closeClient();
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
    printf("\nВыход из чата\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(1);
}

//добавление нового сообщения в массив
void addNewMessage(char *message) {
    if (nextFreeString < 1023) {
        receivedMessenges[nextFreeString] = strdup(message);
        nextFreeString++;
    }
}

//вывод на экран сообщений, которые пришли пока я писала
void printReceivedMessenges() {
    int count = 0;
    while (count < nextFreeString) {
        printf("%s\n", receivedMessenges[count]);
        free(receivedMessenges[count]);
        count++;
    }
    nextFreeString = 0;
}

char *readMessagee() {
    int size;
    //считываю длину сообщения - 1 байт
    if (readN(sockfd, &size, sizeof(int)) <= 0) {
        perror("ERROR reading from socket");
        close(sockfd);
    }

    char *buffer = (char *) malloc(size * sizeof(char));

    //считываю остальное сообщение
    if (readN(sockfd, buffer, size) <= 0) {
        perror("ERROR reading from socket");
        close(sockfd);
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