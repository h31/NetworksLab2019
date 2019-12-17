#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <bits/fcntl-linux.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define PORT 5001;

//Структура для хранения клиента
typedef struct Client {
    int fd;
    int flag;
    char *name;
    struct Client *next;
} ClientLinkedList;

//Функция добавления нового клиента по файловому дескриптору.
//Инициализирует экземпляр клиента и возвращает его
ClientLinkedList *newClient(int fd) {
    ClientLinkedList *temp = (ClientLinkedList *) malloc(sizeof(ClientLinkedList));
    temp->fd = fd;
    temp->flag = 0;
    temp->next = NULL;
    return temp;
}

//Ищет клиента в связном списке, начиная с узла root и продвигаясь
//дальше по списке
ClientLinkedList *findClientByFd(int fd, ClientLinkedList *root) {
    ClientLinkedList *temp = root;
    while (temp->next != NULL) {
        if (temp->fd == fd) {
            return temp;
        }
        temp = temp->next;
    }
    if (temp->fd == fd) {
        return temp;
    }
}

void buildCorrectString(char *str) {
    for (int i = 0; i < (int) strlen(str); i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}

//Считывает байты из дескриптора, пока не будет считана определённая длина.
//Это обусловленно тем, что функция read по факту может прочитать меньше байт,
//чем было запрошено, так как остальная часть ещё не успела к нам дойти.
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
            perror("Ошибка чтения из сокета!");
            exit(1);
        }
        length -= res;
        readSize += res;
    }
    return readSize;
}

char *getCurrentTime() {
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

void sendContentToClient(int fd, char *buffer, uint32_t bufferSize);
uint32_t getContentSize(int fd);
char *readContent(int fd, char *message, char *buffer, uint32_t bufferSize);
void clientDisconnected(ClientLinkedList *disconnectedClient);
void initConnection();
void initServerPoll();
void bindListener();
void deletePollFd(int fd);
void incrementFdPointer();
void decrementFdPointer();
void shutdownServer();

//Указатели на конец и начало связного списка, в котором будут хранится наши клиенты
ClientLinkedList *first, *last;
//Сокет для приёма новых клиентов
int mainSocket;
//Количество дескрипторов в нашем poll
int pollSize;
//Собственно указатель на дескрипторы
struct pollfd *fds;

int main(int argc, char *argv[]) {
    //Длина адреса клиента
    unsigned int clientAddressLength;
    //Адрес клиента
    struct sockaddr_in clientAddress;

    int temporarySize;
    //переменная для проверки значений, возвращаемых различными системными функциями
    int operationCode;
    //Указатели на строки буфера и сообщения
    char *buffer, *message;
    //Размер буфера
    uint32_t bufferSize;

    //Задаём начальное значение, которое позже будет увеличиваться по мере
    //подключения клиентов
    pollSize = 1;

    if (signal(SIGINT, shutdownServer) == SIG_ERR) {
        perror("Ошибка в sigint_handler");
        exit(1);
    }

    //Инициализируем всё, необходимое для работы
    initConnection();
    bindListener();
    clientAddressLength = sizeof(clientAddress);
    first = newClient(mainSocket);
    last = first;
    fds = (struct pollfd *) malloc(sizeof(struct pollfd));
    initServerPoll();

    //Цикл, в котором мы принимаем новых клиентов
    while (1) {
        operationCode = poll(fds, pollSize, -1);
        if (operationCode < 0) {
            perror("Ошибка при использовании poll");
            break;
        }

        temporarySize = pollSize;
        for (int i = 0; i < temporarySize; i++) {

            if (fds[i].revents == 0) {
                continue;
            }

            if (fds[i].revents != POLLIN) {
                printf("loop: %d\n", i);
                perror("Некорректное состояние revents\n");
                shutdownServer();
            }

            if (fds[i].fd == mainSocket) {
                //accept client
                while (1) {
                    operationCode = accept(mainSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);
                    if (operationCode < 0) {
                        break;
                    }

                    incrementFdPointer();
                    fds[pollSize - 1].fd = operationCode;
                    fds[pollSize - 1].events = POLLIN;

                    ClientLinkedList *client = newClient(operationCode);
                    last->next = client;
                    last = client;

                }
            } else {
                //read message size
                bufferSize = getContentSize(fds[i].fd);
                buffer = (char *) malloc(bufferSize);
                message = (char *) malloc(bufferSize + 45);

                int temp_fd = fds[i].fd;

                //wait for message income
                operationCode = poll(&fds[i], 1, -1);
                if (operationCode < 0) {
                    perror("Ошибка при использовании poll");
                    exit(1);
                }
                if (fds[i].revents != POLLIN) {
                    printf("Некорректное состояние revents");
                    exit(1);
                }

                //read message
                strcpy(message, readContent(fds[i].fd, message, buffer, bufferSize));

                for (int j = 0; j < pollSize; j++) {
                    if (fds[j].fd != mainSocket && fds[j].fd != temp_fd) {
                        sendContentToClient(fds[j].fd, message, bufferSize + 45);
                    }
                }
                free(buffer);
                free(message);
            }
        }
    }
    shutdownServer();
    return 0;
}

//initializing server nonblock socket
void initConnection() {
    int on = 1;

    mainSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (mainSocket < 0) {
        perror("Ошибка открытия сокета");
        shutdownServer();
    }

    if (setsockopt(mainSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0) {
        perror("Ошибка при использовании setsockopt");
        shutdownServer();
    }

    if (fcntl(mainSocket, F_SETFL, O_NONBLOCK) < 0) {
        perror("Ошибка при создании неблокирующего сокета");
        shutdownServer();
    }
}

//initializing server socket pollfd
void initServerPoll() {
    bzero(fds, sizeof(fds));
    fds[0].fd = mainSocket;
    fds[0].events = POLLIN;
}

//binding socket and listen
void bindListener() {
    uint16_t portNumber;
    struct sockaddr_in serverAddress;

    portNumber = PORT

    bzero((char *) &serverAddress, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumber);

    if (bind(mainSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("Ошибка при использовании bind");
        shutdownServer();
    }

    if (listen(mainSocket, 5) < 0) {
        perror("Ошибка при использовании listen");
        shutdownServer();
    }
}

//writing message to one client
void sendContentToClient(int fd, char *buffer, uint32_t bufferSize) {
    if (write(fd, &bufferSize, sizeof(int)) < 0) {
        if (errno != EWOULDBLOCK) {
            perror("Ошибка использования сокета");
            shutdownServer();
        }
    }
    if (write(fd, buffer, bufferSize) < 0) {
        if (errno != EWOULDBLOCK) {
            perror("Ошибка использования сокета");
            shutdownServer();
        }
    }
}

//getting size of the message
uint32_t getContentSize(int fd) {
    uint32_t buff_size;
    if (read(fd, &buff_size, sizeof(int)) < 0) {
        if (errno != EWOULDBLOCK) {
            perror("Ошибка чтения из сокета");
            shutdownServer();
        }
    }
    return buff_size;
}

//reading message and preparing it for sending to clients
char *readContent(int fd, char *message, char *buffer, uint32_t bufferSize) {
    int check;
    check = readNBytes(fd, buffer, bufferSize);
    if (check < 0) {
        if (errno != EWOULDBLOCK) {
            perror("Ошибка чтения из сокета");
            shutdownServer();
        }
    }

    ClientLinkedList *client = findClientByFd(fd, first);

    if (check == 0 || !strcmp(buffer, "/exit")) {
        printf("Клиент \"%s\" отключился\n", client->name);
        sprintf(message, "Клиент \"%s\" отключился\n", client->name);
        deletePollFd(client->fd);
        clientDisconnected(findClientByFd(fd, first));
    } else if (!client->flag) {
        client->name = (char *) malloc(bufferSize);
        strcpy(client->name, buffer);
        printf("Клиент \"%s\" подключился\n", buffer);
        sprintf(message, "Клиент \"%s\" подключился\n", buffer);
        client->flag = 1;
    } else {
        printf("Сообщение от %s: %s\n", client->name, buffer);
        sprintf(message, "[%s] %s: %s\n", getCurrentTime(), client->name, buffer);
    }

    return message;
}

//handling client disconnection
void clientDisconnected(ClientLinkedList *disconnectedClient) {
    close(disconnectedClient->fd);
    ClientLinkedList *temp = first;
    while (temp->next != disconnectedClient) {
        temp = temp->next;
    }
    if (disconnectedClient->next == NULL && first->next == disconnectedClient) {
        close(mainSocket);
        printf("Все пользоватили покинули чат. Доброй ночи!\n");
        first->next = NULL;
        last = first;
        free(disconnectedClient);
        initConnection();
        bindListener();
        initServerPoll();
    } else if (disconnectedClient->next == NULL) {
        last = temp;
        temp->next = NULL;
        free(disconnectedClient);
    } else {
        temp->next = disconnectedClient->next;
        free(disconnectedClient);
    }
}

//delete one pollfd from array
void deletePollFd(int fd) {
    for (int i = 0; i < pollSize; i++) {
        if (fds[i].fd == fd) {
            for (int j = i; j < pollSize; j++) {
                if (j != pollSize - 1) {
                    fds[j] = fds[j + 1];
                }
            }
            decrementFdPointer();
            break;
        }
    }
}

void incrementFdPointer() {
    pollSize++;
    fds = (struct pollfd *) realloc(fds, pollSize * sizeof(struct pollfd));
}

void decrementFdPointer() {
    pollSize--;
    fds = (struct pollfd *) realloc(fds, pollSize * sizeof(struct pollfd));
}

void shutdownServer() {
    while (first->next != NULL) {
        clientDisconnected(first->next);
    }
    close(mainSocket);
    printf("\n");
    exit(EXIT_SUCCESS);
}
