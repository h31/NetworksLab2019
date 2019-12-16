#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <signal.h>

#define GET_LIST_OF_TOPICS 1
#define GET_LIST_OF_NEWS 2
#define GET_NEWS 3
#define ADD_TOPIC 4
#define ADD_NEWS 5
#define LIST_OF_TOPICS 6
#define LIST_OF_NEWS 7
#define NEWS 8
#define ERROR 9
#define ACK 10

typedef struct list {
    int socket; // поле данных
    struct list *next; // указатель на следующий элемент
    struct list *prev; // указатель на предыдущий элемент
} clientSocket;

typedef struct news {
    char *news_header; // заголовок новости
    char *news_body;   // текст новости
    struct news *next; // указатель на следующий элемент
    struct news *prev; // указатель на предыдущий элемент
} news;

typedef struct topic {
    char *topic_name; // название темы
    news *first_news; // ссылка на первую новость
    int headers_length; //длина всех заголовков новстей этой темы
    struct topic *next; // указатель на следующий элемент
    struct topic *prev; // указатель на предыдущий элемент
} topic;

int topicsLength = 0;
clientSocket *firstClient = NULL;
topic *firstTopic = NULL;

void sigHandler(int sig);

void error(clientSocket *clientStruct, uint16_t type, char *error);

void ack(clientSocket *clientStruct, uint16_t type);

void getListOfTopics(clientSocket *clientStruct);

void closeSocket(clientSocket *socket);

void addTopic(char *buffer, clientSocket *clientStruct);

void addNews(char *buffer, clientSocket *clientStruct);

void *newClientFunc(void *clientStruct);

char *readMessage(clientSocket *socket, int *sz);

int readN(int socket, void *buf, int length);

void getListOfNews(char *buffer,clientSocket *clientStruct);

void getNews(char *buffer, clientSocket *clientStruct);

int sockfd, packet_length;
pthread_mutex_t mutex;
uint16_t opcode;

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
    portno = (uint16_t) atoi(argv[1]);
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
            firstClient = newClient;
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
        while (tmpClient != NULL) {
            shutdown(tmpClient->socket, SHUT_RDWR);
            close(tmpClient->socket);
            tmpClient = tmpClient->next;
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
    char *buffer;
    int sz;
    while (1) {
        //для его длины
        sz = 0;
        buffer = readMessage((clientSocket *) clientStruct, &sz);
        opcode = *(uint16_t *) buffer;
        switch (opcode) {
            case ADD_TOPIC:
                addTopic(buffer, (clientSocket *) clientStruct);
                break;
            case GET_LIST_OF_TOPICS:
                getListOfTopics((clientSocket *) clientStruct);
                break;
            case ADD_NEWS:
                addNews(buffer, (clientSocket *) clientStruct);
                break;
            case GET_LIST_OF_NEWS:
                getListOfNews(buffer, (clientSocket *) clientStruct);
                break;
            case GET_NEWS:
                getNews(buffer,(clientSocket *) clientStruct);
                break;
            default:
                printf("Error\n");
                break;
        }
        free(buffer);
    }

}

void getListOfTopics(clientSocket *clientStruct) {
    packet_length = 6 + topicsLength;
    char packet[packet_length];
    bzero(packet, packet_length);
    opcode = LIST_OF_TOPICS;
    memcpy(packet, &packet_length, 4);
    memcpy(packet + 4, &opcode, 2);

    if (topicsLength != 0) {
        int pointer = 6;
        topic *tmpTopic = firstTopic;
        while (tmpTopic != NULL) {
            memcpy(packet + pointer, tmpTopic->topic_name, strlen(tmpTopic->topic_name) + 1);
            pointer += (int) strlen(tmpTopic->topic_name) + 1;
            tmpTopic = tmpTopic->next;
        }
    }
    if (write(clientStruct->socket, packet, packet_length) <= 0) {
        closeSocket(clientStruct);
    }
}

void addTopic(char *buffer, clientSocket *clientStruct) {
    //"вечно" первая тема
    topic *tmpTopic = firstTopic;
    //структура для новой темы
    topic *newTopic = (topic *) malloc(sizeof(topic));

    pthread_mutex_lock(&mutex);

    newTopic->next = NULL;
    if (firstTopic == NULL) {
        newTopic->prev = NULL;
        newTopic->topic_name = strdup(buffer + 2);
        newTopic->first_news = NULL;
        firstTopic = newTopic;
        topicsLength += (int) strlen(newTopic->topic_name) + 1;
    } else {
        while (tmpTopic->next != NULL) {
            if (strcmp(tmpTopic->topic_name, buffer+2) == 0) {
                //сообщение об ошибке
                error(clientStruct, 3, "Добавление существующей новостной темы");
                free(newTopic);
                pthread_mutex_unlock(&mutex);
                return;
            }
            tmpTopic = tmpTopic->next;
        }
        newTopic->prev = tmpTopic;
        newTopic->topic_name = strdup(buffer + 2);
        newTopic->first_news = NULL;
        newTopic->headers_length = 0;
        tmpTopic->next = newTopic;
        topicsLength += (int) strlen(newTopic->topic_name) + 1;
    }
    pthread_mutex_unlock(&mutex);
    ack(clientStruct, 1);
}

void findTopic(){
    //перенести сюда
    //динамические массивы
    //чтение и запись в файлы?
}

void getListOfNews(char *buffer, clientSocket *clientStruct) {
    char *newsTopic = buffer + 2;
    //"вечно" первая тема
    topic *tmpTopic = firstTopic;

    pthread_mutex_lock(&mutex);
    while (tmpTopic!= NULL && strcmp(tmpTopic->topic_name,newsTopic)!=0) {
        tmpTopic = tmpTopic->next;
    }
    if(tmpTopic == NULL){
        error(clientStruct, 1, "Отсутствие новостной темы");
        pthread_mutex_unlock(&mutex);
        return;
    }

    packet_length = 6 + tmpTopic->headers_length+ strlen(tmpTopic->topic_name) + 1;
    char packet[packet_length];
    bzero(packet, packet_length);
    opcode = LIST_OF_NEWS;
    memcpy(packet, &packet_length, 4);
    memcpy(packet + 4, &opcode, 2);
    memcpy(packet + 6, tmpTopic->topic_name, strlen(tmpTopic->topic_name) + 1);
    if (tmpTopic->headers_length != 0) {
        int pointer = 6+strlen(tmpTopic->topic_name) + 1;
        news *tmpNews = tmpTopic->first_news;
        while (tmpNews != NULL) {
            memcpy(packet + pointer, tmpNews->news_header, strlen(tmpNews->news_header) + 1);
            pointer += (int) strlen(tmpNews->news_header) + 1;
            tmpNews = tmpNews->next;
        }
    }
    if (write(clientStruct->socket, packet, packet_length) <= 0) {
        closeSocket(clientStruct);
    }
    pthread_mutex_unlock(&mutex);
}

void getNews(char *buffer, clientSocket *clientStruct) {
    char *newsTopic = buffer + 2;
    //"вечно" первая тема
    topic *tmpTopic = firstTopic;
    pthread_mutex_lock(&mutex);
    while (tmpTopic != NULL && strcmp(tmpTopic->topic_name, newsTopic) != 0) {
        tmpTopic = tmpTopic->next;
    }
    if (tmpTopic == NULL) {
        error(clientStruct, 1, "Отсутствие новостной темы");
        pthread_mutex_unlock(&mutex);
        return;
    }

    char *newsHeader = buffer + 2 + strlen(newsTopic) + 1;
    news *tmpNews = tmpTopic->first_news;
    while (tmpNews != NULL && strcmp(tmpNews->news_header, newsHeader) != 0) {
        tmpNews = tmpNews->next;
    }
    if (tmpNews == NULL) {
        error(clientStruct, 2, "Отсутствие новости с данным заголовком");
        pthread_mutex_unlock(&mutex);
        return;
    }
    packet_length =
            6 + strlen(tmpTopic->topic_name) + 1 + strlen(tmpNews->news_header) + 1 + strlen(tmpNews->news_body) + 1;
    char packet[packet_length];
    bzero(packet, packet_length);
    opcode = NEWS;
    memcpy(packet, &packet_length, 4);
    memcpy(packet + 4, &opcode, 2);
    memcpy(packet + 6, tmpTopic->topic_name, strlen(tmpTopic->topic_name) + 1);
    memcpy(packet + strlen(tmpTopic->topic_name) + 1, tmpNews->news_header, strlen(tmpNews->news_header) + 1);
    memcpy(packet + strlen(tmpTopic->topic_name) + 1 + strlen(tmpNews->news_header)+1, tmpNews->news_body, strlen(tmpNews->news_body) + 1);
    pthread_mutex_unlock(&mutex);
    if (write(clientStruct->socket, packet, packet_length) <= 0) {
        closeSocket(clientStruct);
    }
}

void addNews(char *buffer, clientSocket *clientStruct) {
    char *newsTopic = buffer + 2;
    //"вечно" первая тема
    topic *tmpTopic = firstTopic;

    pthread_mutex_lock(&mutex);
    while (tmpTopic!= NULL && strcmp(tmpTopic->topic_name,newsTopic)!=0) {
        tmpTopic = tmpTopic->next;
    }
    if(tmpTopic == NULL){
        error(clientStruct, 1, "Отсутствие новостной темы");
        pthread_mutex_unlock(&mutex);
        return;
    }
    //если тема существует, добавляем новость

    //первая новость
    news *tmpNews = tmpTopic->first_news;
    //структура для новой темы
    news *newNews = (news *) malloc(sizeof(news));

    newNews->next = NULL;
    if (tmpTopic->first_news == NULL) {
        newNews->prev = NULL;
        newNews->news_header = strdup(buffer+(2+ strlen(newsTopic)+1));
        tmpTopic->headers_length += (int) strlen(newNews->news_header)+1;
        newNews->news_body = strdup(buffer+(2+strlen(newsTopic)+1+strlen(newNews->news_header)+1));
        tmpTopic->first_news = newNews;
    } else {
        while (tmpNews->next != NULL) {
            if (strcmp(tmpNews->news_header, buffer+(2+ strlen(newsTopic)+1)) == 0) {
                //сообщение об ошибке
                error(clientStruct, 4, "Добавление новости с существующим заголовком");
                free(newNews);
                pthread_mutex_unlock(&mutex);
                return;
            }
            tmpNews = tmpNews->next;
        }
        newNews->prev = tmpNews;
        newNews->news_header = strdup(buffer+(2+ strlen(newsTopic)+1));
        tmpTopic->headers_length += (int) strlen(newNews->news_header)+1;
        newNews->news_body = strdup(buffer+(2+strlen(newsTopic)+1+strlen(newNews->news_header)+1));
        tmpNews->next = newNews;
    }
    pthread_mutex_unlock(&mutex);
    ack(clientStruct, 2);
}

void error(clientSocket *clientStruct, uint16_t type, char *error) {
    packet_length = 9 + strlen(error);
    char packet[packet_length];
    opcode = ERROR;
    memcpy(packet, &packet_length, 4);
    memcpy(packet + 4, &opcode, 2);
    memcpy(packet + 6, &type, 2);
    memcpy(packet + 8, error, strlen(error) + 1);

    if (write(clientStruct->socket, packet, packet_length) <= 0) {
        closeSocket(clientStruct);
    }
}

void ack(clientSocket *clientStruct, uint16_t type) {
    packet_length = 8;
    char packet[packet_length];
    opcode = ACK;
    memcpy(packet, &packet_length, 4);
    memcpy(packet + 4, &opcode, 2);
    memcpy(packet + 6, &type, 2);
    if (write(clientStruct->socket, packet, packet_length) <= 0) {
        closeSocket(clientStruct);
    }
}


//закрытие/удаление клиента
void closeSocket(clientSocket *socket) {
    //закрываю сокет
    shutdown(socket->socket, SHUT_RDWR);
    close(socket->socket);
    pthread_mutex_lock(&mutex);
    //удаляю из списка
    if (socket->prev != NULL) {
        socket->prev->next = socket->next;
    }
    if (socket->next != NULL) {
        socket->next->prev = socket->prev;
    }
    pthread_mutex_unlock(&mutex);
    //закрываю поток
    pthread_exit(NULL);
}

char *readMessage(clientSocket *socket, int *sz) {
    char *buffer;
    //считываю длину сообщения - 4 байтa
    if (readN(socket->socket, sz, sizeof(int)) <= 0) {
        perror("ERROR reading from socket\n");
        closeSocket(socket);
    }
    *sz -= 4;
    buffer = malloc(*sz);
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

