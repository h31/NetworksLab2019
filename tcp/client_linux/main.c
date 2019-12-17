#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>

void handleWriting(void *arg);
void handleReading(void *arg);
void initConnection();
void handleConnection(uint16_t portNumber, struct hostent *server);
void shutdownClient();

int mainSocket;

int main(int argc, char *argv[]) {
    //Порт, на который мы будем подключаться
    uint16_t portNumber;
    //Сущность сервера
    struct hostent *server;
    //Имя пользователя
    char *userName;
    size_t temp;
    //Размер буфера
    uint32_t bufferSize;
    //Структура POLL, с помощью которого мы будем общаться с сервером
    struct pollfd fdreed;

    if (argc != 3) {
        fprintf(stderr, "Неправильные аргументы: %s [хост] [порт]\n", argv[0]);
        exit(0);
    }

    //Получаем сервер по аргументу из консоли (адресу)
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Ошибка подключения к хосту\n");
        exit(0);
    }

    if (signal(SIGINT, shutdownClient) == SIG_ERR) {
        perror("Ошибка в sigint_handler");
        exit(1);
    }

    portNumber = (uint16_t) atoi(argv[2]);

    //Инициализируем всё что нужно для подключения
    initConnection();

    //Устанавливаем подключение
    handleConnection(portNumber, server);

    //get name from console
    while (1) {
        userName = NULL;
        temp = 0;
        printf("Введите ваше имя:");
        bufferSize = getline(&userName, &temp, stdin);
        if (bufferSize <= 21) break;
        else printf("Сликом длинное имя! Максимум 20 символов.\n");
    }
    buildCorrectString(userName);
    printf("Имя: %s\n", userName);

    //send name size
    if (write(mainSocket, &bufferSize, sizeof(int)) < 0) {
        perror("Ошибка работы с сокетом");
        exit(1);
    }

    //send name
    if (write(mainSocket, userName, bufferSize) < 0) {
        perror("Ошибка работы с сокетом");
        exit(1);
    }
    printf("Напишите /exit чтобы выйти из чата\n\n");

    fdreed.fd = mainSocket;
    fdreed.events = POLLIN;

    pthread_t sendThread;
    if (pthread_create(&sendThread, NULL, (void *) handleWriting, &mainSocket) != 0) {
        printf("Ошибка создания потока\n");
        exit(1);
    }

    pthread_t readingThread;
    if (pthread_create(&readingThread, NULL, (void *) handleReading, &fdreed) != 0) {
        printf("Ошибка создания потока\n");
    }

    pthread_join(sendThread, NULL);
    pthread_join(readingThread, NULL);
    return 0;
}

//initializing client nonblock socket
void initConnection() {
    mainSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (mainSocket < 0) {
        perror("Ошибка открытия сокета");
        exit(1);
    }

    if (fcntl(mainSocket, F_SETFL, O_NONBLOCK) < 0) {
        perror("Ошибка создания неблокирующего сокета");
        exit(1);
    }
}

//initializing connection
void handleConnection(uint16_t portNumber, struct hostent *server) {
    struct sockaddr_in serverAddress;
    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serverAddress.sin_addr.s_addr, (size_t) server->h_length);
    serverAddress.sin_port = htons(portNumber);

    while (connect(mainSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        if (errno != EINPROGRESS) {
            perror("Ошибка при подключении");
            exit(1);
        }
    }
}

//thread function for sending
void handleWriting(void *arg) {
    int socket = *(int *) arg;
    char *buffer;
    uint32_t bufferSize;
    size_t temp;

    while (1) {
        while (1) {
            buffer = NULL;
            temp = 0;
            printf("Ввод: ");
            bufferSize = getline(&buffer, &temp, stdin);
            if (bufferSize > 1) break;
        }
        buildCorrectString(buffer);

        //send message size
        if (write(socket, &bufferSize, sizeof(int)) < 0) {
            if (errno != EWOULDBLOCK) {
                perror("Ошибка работы с сокетом");
                break;
            }
        }

        //send message
        if (write(socket, buffer, bufferSize) < 0) {
            if (errno != EWOULDBLOCK) {
                perror("Ошибка работы с сокетом");
                break;
            }
        }

        buildCorrectString(buffer);
        if (strcmp(buffer, "/exit") == 0) {
            printf("Отключаюсь.\n");
            shutdownClient();
        }
    }

    shutdownClient();
}

//thread function for reading
void handleReading(void *arg) {
    struct pollfd fdreed = *(struct pollfd *) arg;
    int opperationCode;
    char *buffer;
    uint32_t bufferSize;
    while (1) {
        opperationCode = poll(&fdreed, 1, -1);
        if (opperationCode < 0) {
            perror("Ошибка при использовании poll");
            exit(1);
        }

        if (fdreed.revents == 0) {
            continue;
        }

        if (fdreed.revents != POLLIN) {
            printf("Некорректное состояние revents");
            exit(1);
        }

        //get size of message
        bufferSize = 0;
        opperationCode = read(fdreed.fd, &bufferSize, sizeof(int));
        if (opperationCode < 0) {
            if (errno != EWOULDBLOCK) {
                perror("Ошибка чтения длины сообщения");
                shutdownClient();
            }
        }
        if (opperationCode == 0) {
            printf("\rСервер отключился\n");
            shutdownClient();
        }

        //waiting for message income
        opperationCode = poll(&fdreed, 1, -1);
        if (opperationCode < 0) {
            perror("Ошибка при использовании poll");
            exit(1);
        }
        if (fdreed.revents != POLLIN) {
            printf("Некорректное состояние revents");
            exit(1);
        }

        //get message
        buffer = (char *) malloc(bufferSize);
        if (readNBytes(fdreed.fd, buffer, bufferSize) < 0) {
            if (errno != EWOULDBLOCK) {
                perror("Ошибка чтения сообщения");
                shutdownClient();
            }
        }
        printf("\r");
        printf("%s", buffer);

        printf("Ввод: ");
        fflush(stdout);
        free(buffer);
    }
}

void shutdownClient() {
    close(mainSocket);
    printf("\n");
    exit(EXIT_SUCCESS);
}

void buildCorrectString(char *originString) {
    for (int i = 0; i < (int) strlen(originString); i++) {
        if (originString[i] == '\n') {
            originString[i] = '\0';
            break;
        }
    }
}

int readNBytes(int fd, char *buffer, int length) {
    int readSize = 0;
    int res;
    while (readSize < length) {
        res = read(fd, buffer + readSize, length);
        if (res == 0) {
            readSize = res;
            break;
        }
        if (res < 0) {
            perror("Ошибка чтения из сокета");
            exit(1);
        }
        length -= res;
        readSize += res;
    }
    return readSize;
}
