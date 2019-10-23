#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>

int sendContent(int destination, char *content);

void *readingMessages(void *args);

void *sendingMessages(void *arg);

char *readMessage(int fromSock);

int readN(int socket, char *buf, int length);

void removeNewLines(char *str);

/*
 * Основная функция для старта сервера
 * */
int main(int argc, char *argv[]) {

    printf("Client started with PID = %d\n", getpid());

    //Сокет для соединения с сервером
    int sockfd;
    //Номер порта
    uint16_t portNumber;
    //Адрес сервера
    struct sockaddr_in serverAddress;
    //Информация о сервере
    struct hostent *server;
    //Имя пользователя
    char *nickname;

    //Проверка на количество аргументов
    if (argc != 4) {
        fprintf(stderr, "usage %s hostname port nickname\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //Преобразуем порт из строки в int
    portNumber = (uint16_t) atoi(argv[2]);

    //Открываем сокет
    printf("Открываю сокет\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //Проверяем корректно ли открылся сокет
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    //Получаем информацию о хосте с помощью адреса
    printf("Получаю информацию о хосте\n");
    server = gethostbyname(argv[1]);

    //Проверяем удалось ли получить информацию о хосте
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    //Читаем имя пользователя из аргумента
    printf("Считываю никнейм\n");
    nickname = argv[3];

    //Проверяем не пустое ли имя пользователя
    if (strcmp(nickname, "") == 0) {
        fprintf(stderr, "ERROR, nickname is empty\n");
        exit(EXIT_FAILURE);
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
    if (connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    //Первым делом посылаем имя пользователя
    sendContent(sockfd, nickname);


    //Создаём поток на чтение сообщений
    pthread_t tid_read;
    if (pthread_create(&tid_read, NULL, (void *) readingMessages, &sockfd) != 0) {
        printf("Read thread has not created");
        exit(EXIT_FAILURE);
    }

    //создаём поток на отправку сообщений
    pthread_t tid_send;
    if (pthread_create(&tid_send, NULL, (void *) sendingMessages, &sockfd)) {
        printf("Send thread has not created");
        exit(EXIT_FAILURE);
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
    //Достаём наш сокет из аргументов
    int readSocket = *(int *) arg;

    //Цикл в котором мы читаем входящие сообщения
    for (;;) {
        //Достаём буфер прочтённого сообщения
        char *readBuffer = readMessage(readSocket);
        //Подпираем это всё костылём
        removeNewLines(readBuffer);
        //Выводим полученное сообщение
        printf("\n%s\n", readBuffer);
        fflush(stdout);
        //Отпускаем память нашего буфера, так как больше этот текст нам не нужен
        free(readBuffer);
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

//Метод прочтения одного сообщения
char *readMessage(int fromSock) {
    //Код операции (Если меньше 0 то всё плохо)
    int operationCode;
    //Размер сообщения
    int size;

    //Читаем сначала длину сообщения в переменную size
    operationCode = readN(fromSock, &size, sizeof(int));

    //Проверяем всё ли в порядке с прочтением длины
    if (operationCode < 0) {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }

    //Вдруг нам пришлют MAX_INT, чё мы потом будем делать когда выделим столько памяти
    if (size > 10000) {
        size = 10000;
    }

    //Выделяем память под полученную длину
    char *buffer = (char *) malloc(size);
    //Читаем в этот буфер заранее известное кол-во данных
    operationCode = readN(fromSock, buffer, size);

    //Проверяем всё ли в порядке с чтением сообщения
    if (operationCode < 0) {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }

    //Возвращаем наш великолепный буффер
    return buffer;
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
