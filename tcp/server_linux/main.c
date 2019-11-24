#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>
#include <poll.h>
#include <errno.h>

#define PORT_NUMBER 5001
#define MAX_MESSAGE_SIZE 5000
#define MESSAGE_HEADER_SIZE 60

typedef struct Client {
    int FD;
    char *name;
    int flag;
    struct Client *next;
} ClientsLinkedList;

char *currentTime();

char *readMessage(ClientsLinkedList *client);

int readN(int socket, char *buf, uint32_t length);

int sendAll(char *name, char *buffer);

int sendContent(int destination, char *content);

int printErrorAndExit(char *errorText);

void clientDisconnected(ClientsLinkedList *clientToExit);

ClientsLinkedList *createLinkedList(int sock);

ClientsLinkedList *addClient(int newClientFD);

void shutdownServer();

const char *readAndFormatMessage(int fromFD, char *formatedMessage, char *buffer, int messageSize);

int getMessageSize(int fromFD);

void sendMessageToClient(int clientFD, char *message, int messageSize);

ClientsLinkedList *findClient(int clientFD);

void removeFD(int fdToRemove);

void addFD();

//Связный список клиентов
ClientsLinkedList *first, *last;
//Количество клиентов
int numberOfClients = 0;
//Сокет, на котором мы встречаем клиентов
int sockfd;
struct pollfd *FDs;
//Кол-во fd
int pollSize = 1;

int main(int argc, char *argv[]) {
    //Порт
    uint16_t portNumber;
    //Код для проверок
    int operationCode;
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

    FDs = (struct pollfd *) malloc(sizeof(struct pollfd));
    bzero(FDs, sizeof(FDs));
    FDs[0].fd = sockfd;
    FDs[0].events = POLLIN;

    for (;;) {
        operationCode = poll(FDs, 1, -1);
        if (operationCode < 0) {
            printf("Ошибка при использовании poll");
            break;
        }

        for (int i = 0; i < pollSize; ++i) {

            if (FDs[i].revents == 0) {
                continue;
            }

            if (FDs[i].revents != POLLIN) {
                printf("loop: %d\n", i);
                perror("ERROR wrong revents\n");
                shutdownServer();
            }

            if (FDs[i].fd == sockfd) {
                //accept client
                while (1) {
                    operationCode = accept(sockfd, (struct sockaddr *) &clientAddress, &clientAddressLength);
                    if (operationCode < 0) {
                        break;
                    }

                    addFD();
                    FDs[pollSize - 1].fd = operationCode;
                    FDs[pollSize - 1].events = POLLIN;

                    ClientsLinkedList *newClient = addClient(operationCode);
                    char *nickName = readMessage(newClient);
                    newClient->name = nickName;

                    if (strcmp(nickName, "") == 0) {
                        perror("Error! Client name should not be empty.");
                        clientDisconnected(newClient);
                    }

                }
            } else {
                //read message size
                int bufferSize = getMessageSize(FDs[i].fd);
                char *messageBuffer = (char *) malloc(bufferSize);
                char *formatedMessage = (char *) malloc(bufferSize + MESSAGE_HEADER_SIZE);

                int tempFD = FDs[i].fd;

                //wait for message income
                operationCode = poll(&FDs[i], 1, -1);
                if (operationCode < 0) {
                    perror("ERROR on poll");
                    exit(1);
                }
                if (FDs[i].revents != POLLIN) {
                    printf("ERROR wrong revents");
                    exit(1);
                }

                //read message
                strcpy(formatedMessage, readAndFormatMessage(FDs[i].fd, formatedMessage, messageBuffer, bufferSize));

                for (int j = 0; j < pollSize; j++) {
                    if (FDs[j].fd != sockfd && FDs[j].fd != tempFD) {
                        sendMessageToClient(FDs[j].fd, formatedMessage, bufferSize + MESSAGE_HEADER_SIZE);
                    }
                }
                free(messageBuffer);
                free(formatedMessage);
            }

        }


    }

    shutdownServer();
    return 0;
}

void addFD() {
    pollSize++;
    FDs = (struct pollfd *) realloc(FDs, pollSize * sizeof(struct pollfd));
}

void removeFD(int fdToRemove) {
    for (int i = 0; i < pollSize; i++) {
        if (FDs[i].fd == fdToRemove) {
            for (int j = i; j < pollSize; j++) {
                if (j != pollSize - 1) {
                    FDs[j] = FDs[j + 1];
                }
            }
            pollSize--;
            FDs = (struct pollfd *) realloc(FDs, pollSize * sizeof(struct pollfd));
            break;
        }
    }
}

void sendMessageToClient(int clientFD, char *message, int messageSize) {
    if (write(clientFD, &messageSize, sizeof(int)) < 0) {
        if (errno != EWOULDBLOCK) {
            perror("ERROR writing to socket");
            shutdownServer();
        }
    }
    if (write(clientFD, message, messageSize) < 0) {
        if (errno != EWOULDBLOCK) {
            perror("ERROR writing to socket");
            shutdownServer();
        }
    }
}

int getMessageSize(int fromFD) {
    uint32_t messageSize;
    if (read(fromFD, &messageSize, sizeof(int)) < 0) {
        if (errno != EWOULDBLOCK) {
            perror("ERROR reading from socket");
            shutdownServer();
        }
    }
    return messageSize;
}

const char *readAndFormatMessage(int fromFD, char *formatedMessage, char *buffer, int messageSize) {
    int check;
    check = readN(fromFD, buffer, messageSize);
    if (check < 0) {
        if (errno != EWOULDBLOCK) {
            perror("ERROR reading from socket");
            shutdownServer();
        }
    }

    ClientsLinkedList *client = findClient(fromFD);

    if (check == 0 || !strcmp(buffer, "/exit")) {
        printf("Client \"%s\" disconnected\n", client->name);
        sprintf(formatedMessage, "Client \"%s\" disconnected\n", client->name);
        removeFD(client->FD);
        clientDisconnected(findClient(fromFD));
    } else if (!client->flag) {
        client->name = (char *) malloc(messageSize);
        strcpy(client->name, buffer);
        printf("Client \"%s\" connected\n", buffer);
        sprintf(formatedMessage, "Client \"%s\" connected\n", buffer);
        client->flag = 1;
    } else {
        printf("message from %s: %s\n", client->name, buffer);
        sprintf(formatedMessage, "<%s> %s: %s\n", currentTime(), client->name, buffer);
    }

    return formatedMessage;
}


ClientsLinkedList *findClient(int clientFD) {
    ClientsLinkedList *pointer = first;
    while (pointer != last) {
        if (pointer->FD == clientFD) {
            return pointer;
        }
        pointer = pointer->next;
    }
    if (pointer->FD == clientFD) {
        return pointer;
    } else {
        return NULL;
    }
}

ClientsLinkedList *addClient(int newClientFD) {
    printf("addClient\n");
    if (numberOfClients < 0) {
        perror("Wait, what the hell?");
        shutdownServer();
    }
    if (numberOfClients == 0) {
        first = createLinkedList(newClientFD);
        last = first;
        numberOfClients++;
        return first;
    }
    ClientsLinkedList *newClient = (ClientsLinkedList *) createLinkedList(newClientFD);
    newClient->FD = newClientFD;
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
    operationCode = readN(client->FD, &size, sizeof(int));

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
    operationCode = readN(client->FD, buffer, size);

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
        sendContent(currentClient->FD, formedMessage);
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
    temp->FD = sock;
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
    close(clientToExit->FD);
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
    exit(1);
}
