#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <sys/poll.h>

#define CLIENTNUMBER 100

void sigHandler(int sig);

void closeSocket(int socketIndex);

void writeToClients(void *msg, void *size);


char *readMessage(int socketIndex, int *sz, char *buffer);

int readN(int socket, void *buf, int length);

int sockfd;
struct pollfd fds[CLIENTNUMBER];

int main() {
    //инициализация
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int count;
    int ret;
    char buffer[256];
    int sz;

    for (int j = 0; j < CLIENTNUMBER; ++j) {
        fds[j].fd = -1;
        fds[j].events = POLLIN;
        fds[j].revents = 0;
    }
    //создание сокета и проверка
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
    }
    fds[0].fd = sockfd;
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

    //отлавливаем закрытие сервера
    signal(SIGINT, sigHandler);


    //ждем новых клиентов
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    //для accept
    int sock;

    while (1) {
        ret = poll(fds, CLIENTNUMBER, 0);
        if (ret == -1) {
            perror("poll error");
            exit(1);
        } else if (ret > 0) {
            if (fds[0].revents == POLLIN) {
                sock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                if (sock < 0) {
                    perror("ERROR on accept");
                    exit(1);
                }
                count = 1;
                while (fds[count].fd != -1 && count < CLIENTNUMBER) {
                    count++;
                }
                if (count < CLIENTNUMBER) {
                    fds[count].fd = sock;
                } else {
                    printf("Превышено количество клиентов");
                }
            }
            for (int i = 1; i < CLIENTNUMBER; ++i) {
                if (fds[i].revents == POLLIN) {
                    bzero(buffer, 256);
                    sz = 0;
                    readMessage(i, &sz, buffer);
                    if(sz > 0) {
                        writeToClients(buffer, &sz);
                    }
                }
            }
        }
    }
}

//обработчик закрытия сервера
void sigHandler(int sig) {
    if (sig != SIGINT) return;
    else {
        printf("\nСервер остановлен\n");
        //закрываем всех клиентов
        for (int i = 0; i < CLIENTNUMBER; ++i) {
            if (fds[i].fd != -1) {
                shutdown(fds[i].fd, SHUT_RDWR);
                close(fds[i].fd);
            }
        }
        exit(1);
    }
}


//рассылка пришедшего сообщения остальным клиентам
void writeToClients(void *msg, void *size) {
    int count = 1;
    while (count != CLIENTNUMBER) {
        if (fds[count].fd != -1) {
            if (write(fds[count].fd, size, sizeof(int)) <= 0) {
                closeSocket(count);
            }
            if (write(fds[count].fd, msg, strlen(msg)) <= 0) {
                closeSocket(count);
            }
        }
        count++;
    }
}


//закрытие/удаление клиента
void closeSocket(int socketIndex) {
    //закрываю сокет
    shutdown(fds[socketIndex].fd, SHUT_RDWR);
    close(fds[socketIndex].fd);
    //удаляю из списка
    fds[socketIndex].fd = -1;
    fds[socketIndex].revents = 0;
}

char *readMessage(int socketIndex, int *sz, char *buffer) {
    //считываю длину сообщения - 4 байтф
    if (readN(fds[socketIndex].fd, sz, sizeof(int)) <= 0) {
        perror("ERROR reading from socket\n");
        closeSocket(socketIndex);
    }
    //считываю остальное сообщение
    if (readN(fds[socketIndex].fd, buffer, *sz) <= 0) {
        perror("ERROR reading from socket\n");
        closeSocket(socketIndex);
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