#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <signal.h>

#define MAX_CLIENTS 5

void sigHandler(int sig);

void closeSocket(int i);

void writeToClients(void *msg, int number, void *size);

void *newClient(void *numb);

char *readMessage(int numb, int *sz, char* buffer);

int readN(int socket, void *buf, int length);

int newsockfd[MAX_CLIENTS];
pthread_t tid[MAX_CLIENTS];
int sockfd;

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

    //привязка сокета к адресу и проверка(+переиспользование адреса для сокета)
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) {
            perror("setsockopt(SO_REUSEADDR) failed");
            exit(1);
        }
    }

    printf("Сервер запущен\n");
    //обнуление массива для хранения клиентов
    bzero(newsockfd, MAX_CLIENTS);

    //отлавливаем закрытие сервера
    signal(SIGINT, sigHandler);

    //ждем новых клиентов
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (1) {
        //принимаем клиента и запоминаем его(проверка того, что соединение прошло успешно)
        newsockfd[i] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd[i] < 0) {
            perror("ERROR on accept");
            exit(1);
        }
        //под каждого клиента свой поток(функция обработчик - newClient)
        if (pthread_create(&tid[i], NULL, newClient, (void *)(uintptr_t) i) < 0) {
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
        int count = 0;
        //закрываем всех клиентов
        while (count < MAX_CLIENTS && newsockfd[count] != 0) {
            shutdown(newsockfd[count], SHUT_RDWR);
            close(newsockfd[count]);
            count++;
        }
        //закрываем главный сокет
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        exit(1);
    }
}

//поток каждого нового клиента, слушаем
void *newClient(void *numb) {
    char buffer[256];
    int sz;
    while (1) {
        bzero(buffer,256);
        sz=0;
        readMessage((uintptr_t) numb, &sz, buffer);

        //рассылаем всем принятое сообщение
        writeToClients(buffer, newsockfd[(uintptr_t) numb], &sz);
    }
}

//рассылка пришедшего сообщения остальным клиентам
void writeToClients(void *msg, int number, void *size) {
    int count = 0;
    //всем клиентам в массиве
    printf("\nOтправляю%s\n", (char*) msg);
    while (count < MAX_CLIENTS && newsockfd[count] != 0) {
        //кроме отправителя
        if (newsockfd[count] != number) {
            if (write(newsockfd[count], size, sizeof(int)) <= 0) {
                closeSocket(count);
            }
            if (write(newsockfd[count], msg, strlen(msg)) <= 0) {
                closeSocket(count);
            }
        }
        count++;
    }
}


//закрытие/удаление клиента
void closeSocket(int i) {
    //закрываю сокет
    shutdown(newsockfd[i], SHUT_RDWR);
    close(newsockfd[i]);

    //подчищаем в массивах данные
    tid[i] = 0;
    newsockfd[i] = 0;
    //закрываю поток
    pthread_exit(&tid[i]);
}

char *readMessage(int numb, int *sz, char* buffer) {
    //считываю длину сообщения - 1 байт
    if (readN(newsockfd[numb], sz, sizeof(int)) <= 0) {
        perror("ERROR reading from socket");
        closeSocket(numb);
    }

    //считываю остальное сообщение
    if (readN(newsockfd[numb], buffer, *sz) <= 0) {
        perror("ERROR reading from socket");
        closeSocket((uintptr_t) numb);
    }
    printf("прочитал %s\n",buffer);
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