#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>

#define MAX_MESSAGE_SIZE 5000
#define h_addr h_addr_list[0]

int sendContent(int destination, char *content);

void *readingMessages(void *args);

void *sendingMessages(void *arg);

char *readMessage(int fromSock);

int readN(int socket, char *buf, int length);

void removeNewLines(char *str);

int main(int argc, char *argv[]) {

    printf("Client started with PID = %d\n", getpid());

    int sockfd;
    uint16_t portNumber;
    struct sockaddr_in serverAddress;
    struct hostent *server;
    char *nickname;

    if (argc != 4) {
        fprintf(stderr, "usage %s hostname port nickname\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    portNumber = (uint16_t) atoi(argv[2]);

    printf("Открываю сокет\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    printf("Получаю информацию о хосте\n");
    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    printf("Считываю никнейм\n");
    nickname = argv[3];

    if (strcmp(nickname, "") == 0) {
        fprintf(stderr, "ERROR, nickname is empty\n");
        exit(EXIT_FAILURE);
    }

    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    printf("Копирую адрес\n");
    bcopy(server->h_addr, (char *) &serverAddress.sin_addr.s_addr, (size_t) server->h_length);
    serverAddress.sin_port = htons(portNumber);

    printf("Подключаюсь к серверу\n");
    if (connect(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    sendContent(sockfd, nickname);

    pthread_t tid_read;
    if (pthread_create(&tid_read, NULL, (void *) readingMessages, &sockfd) != 0) {
        printf("Read thread has not created");
        exit(EXIT_FAILURE);
    }

    pthread_t tid_send;
    if (pthread_create(&tid_send, NULL, (void *) sendingMessages, &sockfd)) {
        printf("Send thread has not created");
        exit(EXIT_FAILURE);
    }

    pthread_join(tid_read, NULL);
    pthread_join(tid_send, NULL);
    return EXIT_SUCCESS;
}

void removeNewLines(char *str) {
    for (int i = 0; i < (int) strlen(str); i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}

void *sendingMessages(void *arg) {
    int writeSocket = *(int *) arg;
    char *writeBuffer;
    size_t bufferLength;

    while (1) {
        writeBuffer = NULL;
        bufferLength = 0;
        getline(&writeBuffer, &bufferLength, stdin);
        sendContent(writeSocket, writeBuffer);
        removeNewLines(writeBuffer);
        if (strcmp(writeBuffer, "/exit") == 0) {
            printf("Goodbye\n");
            close(writeSocket);
            exit(EXIT_SUCCESS);
        }
    }
}

void *readingMessages(void *arg) {
    int readSocket = *(int *) arg;

    for (;;) {
        char *readBuffer = readMessage(readSocket);
        removeNewLines(readBuffer);
        printf("\n%s\n", readBuffer);
        fflush(stdout);
        free(readBuffer);
    }
}

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

char *readMessage(int fromSock) {
    int operationCode;
    int size;

    operationCode = readN(fromSock, &size, sizeof(int));

    if (operationCode < 0) {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }

    if (size > MAX_MESSAGE_SIZE) {
        size = MAX_MESSAGE_SIZE;
    }

    char *buffer = (char *) malloc(size);
    operationCode = readN(fromSock, buffer, size);

    if (operationCode < 0) {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }

    return buffer;
}

int sendContent(int destination, char *content) {
    int operationCode;
    printf("Отправляю длину сообщения\n");
    int size = strlen(content);
    operationCode = write(destination, &size, sizeof(int));
    if (operationCode <= 0) {
        printf("Ошибка передачи\n");
    }
    printf("Отправляю сообщение\n");
    operationCode = write(destination, content, size);
    if (operationCode <= 0) {
        printf("Ошибка передачи\n");
    }
    return operationCode;
}
