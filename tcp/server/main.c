#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <signal.h>

#define MAX_CLIENTS 5
typedef struct list{
    int socket; // поле данных
    struct list *next; // указатель на следующий элемент
    struct list *prev; // указатель на предыдущий элемент
} clientSocket;

clientSocket *firstClient = NULL;

void sigHandler(int sig);

void closeSocket(clientSocket* socket);

void writeToClients(void *msg, void *size);

void *newClientFunc(void *clientStruct);

char *readMessage(clientSocket* socket, int *sz, char *buffer);

int readN(int socket, void *buf, int length);

int sockfd;
pthread_mutex_t mutex;

int main(int argc, char *argv[]) {
    //инициализация
    uint16_t portno;
    unsigned int clilen;
    int i = 0;
    struct sockaddr_in serv_addr, cli_addr;

    //создание сокета и проверка
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
    }

    //инициализация структуры сокета
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = (uint16_t) 5001;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }

    //привязка сокета к адресу и проверка(+переиспользование адреса для сокета)
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    printf("Сервер запущен\n");

    //отлавливаем закрытие сервера
    signal(SIGINT, sigHandler);

    //Инициализация мьютекса
    pthread_mutex_init(&mutex, NULL);

    //ждем новых клиентов
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    //для accept
    int sock;
    //для pthread_create
    pthread_t tid;
    //"вечно" первый элемент
    clientSocket *tmpClient;
    while (1) {
        tmpClient = firstClient;
        //принимаем клиента и запоминаем его(проверка того, что соединение прошло успешно)
        sock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (sock < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        clientSocket *newClient = (clientSocket *) malloc(sizeof(clientSocket));
        pthread_mutex_lock(&mutex);
        newClient->socket = sock;
        newClient->next = NULL;
        if (firstClient == NULL) {
            newClient->prev = NULL;
            firstClient=newClient;
        } else {
            while (tmpClient->next != NULL) {
                tmpClient = tmpClient->next;
            }
            newClient->prev = tmpClient;
            tmpClient->next = newClient;
        }
        pthread_mutex_unlock(&mutex);
        //под каждого клиента свой поток(функция обработчик - newClient)
        if (pthread_create(&tid, NULL, newClientFunc, (void *) newClient) < 0) {
            perror("ERROR on create phread");
            exit(1);
        }
        i++;
    }
}

//обработчик закрытия сервера
void sigHandler(int sig) {
    if (sig != SIGINT) return;
    else {
        printf("\nСервер остановлен\n");
        //закрываем всех клиентов
        clientSocket *tmpClient = firstClient;
        while (tmpClient != NULL){
            shutdown(tmpClient->socket, SHUT_RDWR);
            close(tmpClient->socket);
            tmpClient = tmpClient ->next;
        }

        //закрываем главный сокет
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);

        //Уничтожение мьютекса
        pthread_mutex_destroy(&mutex);
        exit(1);
    }
}

//поток каждого нового клиента, слушаем
void *newClientFunc(void *clientStruct) {
    char buffer[256];
    int sz;
    while (1) {
        //для сообщения
        bzero(buffer, 256);
        //для его длины
        sz = 0;
        readMessage((clientSocket*)clientStruct, &sz, buffer);
        //рассылаем всем принятое сообщение
        writeToClients(buffer , &sz);
    }
}

//рассылка пришедшего сообщения остальным клиентам
void writeToClients(void *msg, void *size) {
    //всем клиентам в массиве
    clientSocket *tmpClient = firstClient;
    pthread_mutex_lock(&mutex);
    while (tmpClient != NULL){
        if (write(tmpClient->socket, size, sizeof(int)) <= 0) {
            closeSocket(tmpClient);
        }
        if (write(tmpClient->socket, msg, strlen(msg)) <= 0) {
            closeSocket(tmpClient);
        }
        tmpClient = tmpClient ->next;
    }
    pthread_mutex_unlock(&mutex);
}


//закрытие/удаление клиента
void closeSocket(clientSocket* socket) {
    //закрываю сокет
    shutdown(socket ->socket, SHUT_RDWR);
    close(socket->socket);
    pthread_mutex_lock(&mutex);
    //удаляю из списка
    if(socket->prev !=NULL){
        socket->prev->next = socket->next;
    }
    if(socket->next!=NULL){
        socket->next->prev = socket->prev;
    }
    pthread_mutex_unlock(&mutex);
    //закрываю поток
    pthread_exit(NULL);
}

char *readMessage(clientSocket* socket, int *sz, char *buffer) {
    //считываю длину сообщения - 4 байтф
    if (readN(socket ->socket, sz, sizeof(int)) <= 0) {
        perror("ERROR reading from socket\n");
        closeSocket(socket);
    }
    //считываю остальное сообщение
    if (readN(socket->socket, buffer, *sz) <= 0) {
        perror("ERROR reading from socket\n");
        closeSocket(socket);
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