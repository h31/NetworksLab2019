#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>

#define PORT_NUMBER 5001
#define MAX_MESSAGE_SIZE 5000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Client {
    int sock;
    char *name;
    struct Client *next;
} ClientsLinkedList;

char *currentTime();

void *handleClient(void *args);

char *readMessage(ClientsLinkedList *client);

int readN(int socket, char *buf, uint32_t length);

int sendAll(char *name, char *buffer);

int sendContent(int destination, char *content);

int printErrorAndExit(char *errorText);

void clientDisconnected(ClientsLinkedList *clientToExit);

ClientsLinkedList *createLinkedList(int sock);

ClientsLinkedList *addClient(int newsockfd);

void shutdownServer();

//Связный список клиентов
ClientsLinkedList *first, *last;
//Количество клиентов
int numberOfClients = 0;
//Сокет, на котором мы встречаем клиентов
int sockfd;

int main(int argc, char *argv[]) {
    //Порт
    uint16_t portNumber;
    //Адреса сервера и клиента
    struct sockaddr_in serverAddress, clientAddress;

    /* Открытие сокета */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printErrorAndExit("Ошибка при открытии сокета");
    }

    /* Инициализируем структуру сокета */
    bzero((char *) &serverAddress, sizeof(serverAddress));
    portNumber = PORT_NUMBER;

    //Задаём семейство адресов
    serverAddress.sin_family = AF_INET;
    //мда
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    //ну тут всё понятно
    serverAddress.sin_port = htons(portNumber);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    //
    listen(sockfd, 5);
    socklen_t clientAddressLength = sizeof(clientAddress);

    int newClientSocket;

    for (;;) {
        /* Accept actual connection from the client */
        newClientSocket = accept(sockfd, (struct sockaddr *) &clientAddress, &clientAddressLength);

        if (newClientSocket < 0) {
            perror("ERROR on accept");
            break;
        }

        ClientsLinkedList *newClient = addClient(newClientSocket);
        char *nickName = readMessage(newClient);
        newClient->name = nickName;

        if (strcmp(nickName, "") == 0) {
            perror("Error! Client name should not be empty.");
            clientDisconnected(newClient);
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, handleClient, (void *) newClient) != 0) {
            printf("thread has not created");
            exit(1);
        }
    }

    shutdownServer();
    return 0;
}

ClientsLinkedList *addClient(int newsockfd) {
    printf("addClient\n");
    if (numberOfClients < 0) {
        perror("Wait, what the hell?");
        shutdownServer();
    }
    if (numberOfClients == 0) {
        first = createLinkedList(newsockfd);
        last = first;
        numberOfClients++;
        return first;
    }
    ClientsLinkedList *newClient = (ClientsLinkedList *) createLinkedList(newsockfd);
    newClient->sock = newsockfd;
    newClient->next = NULL;
    last->next = newClient;
    last = newClient;
    numberOfClients++;
    return newClient;
}

void shutdownServer() {
    printf("shutdownServer\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

}

int readN(int socket, char *buf, uint32_t length) {
    printf("readN\n");
    int result = 0;
    uint32_t readedBytes = 0;
    uint32_t sizeMsg = length;
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

int printErrorAndExit(char *errorText) {
    perror(errorText);
    exit(1);
}

char *readMessage(ClientsLinkedList *client) {
    printf("readMessage\n");
    //Код операции (Если меньше 0 то всё плохо)
    int operationCode;
    //Размер сообщения
    int size;

    //Читаем сначала длину сообщения в переменную size
    operationCode = readN(client->sock, &size, sizeof(int));

    //Проверяем всё ли в порядке с прочтением длины
    printf("Проверяем всё ли в порядке с прочтением длины: %d\n", operationCode);
    if (operationCode < 0) {
        perror("ERROR reading from socket");
        clientDisconnected(client);

        exit(EXIT_FAILURE);
    }

    if (operationCode == 0) {
        perror("HE JUST LEFT AND SAID NOTHING!!!!\n");
        clientDisconnected(client);

        return NULL;
    }

    //Вдруг нам пришлют MAX_INT, чё мы потом будем делать когда выделим столько памяти
    if (size > MAX_MESSAGE_SIZE) {
        size = MAX_MESSAGE_SIZE;
    }

    printf("Выделяем память и читаем само сообщение\n");
    //Выделяем память под полученную длину
    char *buffer = (char *) malloc(size);
    //Читаем в этот буфер заранее известное кол-во данных
    operationCode = readN(client->sock, buffer, size);

    printf("Проверяем всё ли в порядке с чтением сообщения: %d", operationCode);
    //Проверяем всё ли в порядке с чтением сообщения
    if (operationCode < 0) {
        perror("ERROR reading from socket\n");

        exit(EXIT_FAILURE);
    }

    //Возвращаем наш великолепный буффер
    return buffer;
}

int sendAll(char *name, char *buffer) {
    printf("sendAll\n");
    char formedMessage[MAX_MESSAGE_SIZE] = {0};
    sprintf(formedMessage, "<%s> : %s", name, buffer);
    formedMessage[MAX_MESSAGE_SIZE - 1] = '\0';
    ClientsLinkedList *currentClient = first;
    for (int i = 0; i < numberOfClients; ++i) {
        if (strcmp(currentClient->name, name) == 0) {
            continue;
        }
        sendContent(currentClient->sock, formedMessage);
        currentClient = currentClient->next;
    }
}

int sendContent(int destination, char *content) {
    printf("sendContent\n");
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

void *handleClient(void *arg) {
    printf("handleClient\n");
    ClientsLinkedList *client = (ClientsLinkedList *) arg;

    char *buffer;
    char *time;

    for (;;) {
        buffer = readMessage(client);
        if (buffer == NULL) {
            break;
        }
        time = currentTime();
        sendAll(client->name, buffer);
        printf("%s, <%s> %s\n", time, client->name, buffer);
        fflush(stdout);
    }
}

ClientsLinkedList *createLinkedList(int sock) {
    printf("createLinkedList\n");
    ClientsLinkedList *temp = (ClientsLinkedList *) malloc(sizeof(ClientsLinkedList));
    temp->sock = sock;
    temp->name = NULL;
    temp->next = NULL;
    return temp;
}

char *currentTime() {
    printf("currentTime\n");
    time_t timer = time(NULL);
    struct tm *t;
    char tmp[6];
    char *str;
    t = localtime(&timer);
    bzero(tmp, 6);
    strftime(tmp, 6, "%H:%M", t);
    str = (char *) malloc(sizeof(tmp));
    strcpy(str, tmp);
    return str;
}

void clientDisconnected(ClientsLinkedList *clientToExit) {
    printf("clientDisconnected\n");
    close(clientToExit->sock);
    if (first == clientToExit) {
        if (numberOfClients == 1) {
            first = NULL;
            last = NULL;
        } else {
            ClientsLinkedList *secondClient = clientToExit->next;
            first = secondClient;
        }
    } else {
        ClientsLinkedList *listPointer = first;
        while (listPointer->next != clientToExit) {
            listPointer = listPointer->next;
        }
        if (clientToExit->next == NULL) {
            last = listPointer;
            listPointer->next = NULL;
        } else {
            listPointer->next = clientToExit->next;
        }
    }
    numberOfClients--;
    free(clientToExit);
    pthread_exit(NULL);
}
