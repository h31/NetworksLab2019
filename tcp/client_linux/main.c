#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <asm/errno.h>

#define MAX_MESSAGE_SIZE 5000

int sendContent(int destination, char *content);

void *readingMessages(void *args);

void *sendingMessages(void *arg);

int readN(int socket, char *buf, int length);

void removeNewLines(char *str);

//Сокет для соединения с сервером
int sockfd;

/*
 * Основная функция для старта клиента
 * */
int main(int argc, char *argv[]) {

    printf("Client started with PID = %d\n", getpid());
    //Номер порта
    uint16_t portNumber;
    //Адрес сервера
    struct sockaddr_in serverAddress;
    //Информация о сервере
    struct hostent *server;
    //Имя пользователя
    char *nickname;

    struct pollfd fdRead;

    //Проверка на количество аргументов
    if (argc != 4) {
        fprintf(stderr, "usage %s hostname port nickname\n", argv[0]);
        getchar(); exit(EXIT_FAILURE); 
    }

    //Преобразуем порт из строки в int
    portNumber = (uint16_t) atoi(argv[2]);

    //Открываем сокет
    printf("Открываю сокет\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //Проверяем корректно ли открылся сокет
    if (sockfd < 0) {
        perror("ERROR opening socket");
        getchar(); exit(EXIT_FAILURE); 
    }

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        perror("ERROR making socket nonblock");
        getchar(); exit(EXIT_FAILURE); 
    }

    //Получаем информацию о хосте с помощью адреса
    printf("Получаю информацию о хосте\n");
    server = gethostbyname(argv[1]);

    //Проверяем удалось ли получить информацию о хосте
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        getchar(); exit(EXIT_FAILURE); 
    }

    //Читаем имя пользователя из аргумента
    printf("Считываю никнейм\n");
    nickname = argv[3];

    //Проверяем не пустое ли имя пользователя
    if (strcmp(nickname, "") == 0) {
        fprintf(stderr, "ERROR, nickname is empty\n");
        getchar(); exit(EXIT_FAILURE); 
    }

    //На всякий случай обнуляем адрес сервера
    bzero((char *) &serverAddress, sizeof(serverAddress));
    //Задаём семейство адресов сервера
    serverAddress.sin_family = AF_INET;
    //Копируем адрес в переменную сокета
    printf("Копирую адрес\n");
    bcopy(server->h_addr, (char *) &serverAddress.sin_addr.s_addr, (size_t) server->h_length);
    //Определяем сетевой порядок байт для порта
    serverAddress.sin_port = htons(portNumber);

    printf("Подключаюсь к серверу\n");
    //Подключаемся к серверу

    while (connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        //Не представляю как это работает
        //То, что ниже - не работает
    }

//    int errcode;
//    for (;;) {
//        errcode = connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
//        if (errcode > 0){
//            break;
//        }
//        if (errcode == EINPROGRESS) {
//            continue;
//        } else {
//            perror("ERROR connecting");
//            getchar();
//            exit(EXIT_FAILURE);
//        }
//    }

    fdRead.fd = sockfd;
    fdRead.events = POLLIN;

    //Первым делом посылаем имя пользователя
    sendContent(sockfd, nickname);

    //Создаём поток на чтение сообщений
    pthread_t tid_read;
    if (pthread_create(&tid_read, NULL, (void *) readingMessages, &fdRead) != 0) {
        printf("Read thread has not created");
        getchar(); exit(EXIT_FAILURE); 
    }

    //создаём поток на отправку сообщений
    pthread_t tid_send;
    if (pthread_create(&tid_send, NULL, (void *) sendingMessages, &sockfd)) {
        printf("Send thread has not created");
        getchar(); exit(EXIT_FAILURE); 
    }

    //джойним потоки и заканчиваем работу
    pthread_join(tid_read, NULL);
    pthread_join(tid_send, NULL);
    return EXIT_SUCCESS;
}

//Удаляет переносы строк из строки - это фиксит некоторые проблемы, возникшие в ходе работы
void removeNewLines(char *str) {
    for (int i = 0; i < (int) strlen(str); i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}

//Функция для запуска в отдельном потоке, берёт пользовательский ввод и отправляет сообщения на сервер
void *sendingMessages(void *arg) {
    //Достаём сокет из аргументов
    int writeSocket = *(int *) arg;
    //Создаём буфер под наши сообщения
    char *writeBuffer;
    //Заводим переменную под длину буфера
    size_t bufferLength;

    while (1) {
        //Сначала буфер пустой
        writeBuffer = NULL;
        bufferLength = 0;
        //Читаем из консоли ввод пользователя, длина может быть любой
        getline(&writeBuffer, &bufferLength, stdin);
        //Отправляем контент на сервер
        sendContent(writeSocket, writeBuffer);
        //Удаляем из введённой строки лишние переносы
        removeNewLines(writeBuffer);
        //Сравниваем с командой выхода. Если совпадает - собственно вырубаем клиент
        if (strcmp(writeBuffer, "/exit") == 0) {
            printf("Goodbye\n");
            close(writeSocket);
            exit(EXIT_SUCCESS);
        }
    }
}

//Функция для запуска в потоке на чтение сообщений с сервера.
void *readingMessages(void *arg) {
    struct pollfd fdRead = *(struct pollfd *) arg;
    int operationCode;
    char *buffer = NULL;
    size_t bufferSize;

    //Цикл в котором мы читаем входящие сообщения
    for (;;) {
        operationCode = poll(&fdRead, 1, -1);
        if (operationCode < 0) {
            printf("Error using POLL!\n");
            getchar(); exit(EXIT_FAILURE); 
        }
        if (fdRead.revents == 0) {
            continue;
        }
        if (fdRead.revents != POLLIN) {
            printf("Unexpected number in revents");
            getchar(); exit(EXIT_FAILURE); 
        }

        //Размер сообщения
        bufferSize = 0;
        //Читаем сначала длину сообщения в переменную size
        operationCode = readN(fdRead.fd, &bufferSize, sizeof(int));
        if (operationCode < 0) {
            printf("Error reading from socket!\n");
            getchar(); exit(EXIT_FAILURE); 
        }
        if (operationCode == 0) {
            printf("Server is now offline.");
            exit(EXIT_SUCCESS);
        }

        operationCode = poll(&fdRead, 1, -1);
        if (operationCode < 0) {
            printf("Error using POLL!\n");
            getchar(); exit(EXIT_FAILURE); 
        }
        if (fdRead.revents != POLLIN) {
            printf("Unexpected number in revents");
            getchar(); exit(EXIT_FAILURE); 
        }

        //Выделяем память под полученную длину
        buffer = (char *) malloc(bufferSize);
        //Читаем в этот буфер заранее известное кол-во данных
        operationCode = readN(fdRead.fd, buffer, bufferSize);
        if (operationCode < 0) {
            printf("Error reading from socket!\n");
            getchar(); exit(EXIT_FAILURE); 
        }
        if (operationCode == 0) {
            printf("Server is now offline.");
            exit(EXIT_SUCCESS);
        }

        printf("\n%s\n", buffer);
        fflush(stdout);
        free(buffer);
    }
}

//Читаем читаем читаем... пока не прочитаем нужное кол-во данных
int readN(int socket, char *buf, int length) {
    int result = 0;
    int readedBytes = 0;
    int sizeMsg = length;
    while (sizeMsg > 0) {
        readedBytes = read(socket, buf + result, sizeMsg);
        if (readedBytes <= 0) {
            return -1;
        }
        result += readedBytes;
        sizeMsg -= readedBytes;
    }
    return result;
}

//Аналогично readMessage, только операция отправки текста
int sendContent(int destination, char *content) {
    //Код операций
    int operationCode;
    printf("Отправляю длину сообщения\n");
    //Записываем в size длинну сообщения, которую хотим отправить
    int size = strlen(content);
    //Отправляем и сохраняем код операции в operationCode
    operationCode = write(destination, &size, sizeof(int));
    //Проверяем всё ли в порядке с нашей записью
    if (operationCode <= 0) {
        printf("Ошибка передачи\n");
    }
    printf("Отправляю сообщение\n");
    //Теперь вслед передаём само сообщение
    operationCode = write(destination, content, size);
    //Проверяем всё ли нормально прошло
    if (operationCode <= 0) {
        printf("Ошибка передачи\n");
    }
    //Возвращаем код операции
    return operationCode;
}
