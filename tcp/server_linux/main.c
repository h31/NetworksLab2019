#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct Client
{
    int sock;
    char* name;
} clients[10];

int nClients = 0;

int receiveInt (int *num, int fd);
void* handleClient(void* args);
int readMessage(int fromSock, char* buffer);
int readN(int socket, char* buf, int length);
int sendAll(int id, char* name, char* buffer);
int sendContent(int destination, char* content);
int printErrorAndExit(char* errorText);
char *get_time();

int main(int argc, char *argv[]) {
    //Сокет, на котором мы встречаем клиентов
    int sockfd;
    //Порт
    uint16_t portNumber;
    //???
    unsigned int clilen;
    //Адреса сервера и клиента
    struct sockaddr_in serverAddress, clientAddress;
    //???
    ssize_t n;
    //???
    int wordLengthBuffer;

    /* Открытие сокета */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printErrorAndExit("Ошибка при открытии сокета");
    }

    /* Initialize socket structure */
    bzero((char *) &serverAddress, sizeof(serverAddress));
    portNumber = 5001;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNumber);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(clientAddress);

    int newsockfd;
    int client_id;

    for (;;){
        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &clilen);

        if (newsockfd < 0) {
            perror("ERROR on accept");
            break;
        }

        clients[nClients].sock = newsockfd;

        client_id = nClients;

        pthread_t tid;
        if (pthread_create(&tid, NULL, handleClient, (void *) &client_id) != 0) {
            printf("thread has not created");
            exit(1);
        }

        nClients++;

    }
  

    /* Write a response to the client */
    // n = write(newsockfd, "I got your message", 18); // send on Windows

    // if (n < 0) {
    //     perror("ERROR writing to socket");
    //     exit(1);
    // }

//    int true = 1;
//    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    return 0;
}

int readN(int socket, char* buf, int length){
    int result = 0;
    int readedBytes = 0;
    int sizeMsg = length;
    while(sizeMsg > 0){
        readedBytes = read(socket, buf + result, sizeMsg);
        if (readedBytes <= 0){
            return -1;
        }
        result += readedBytes;
        sizeMsg -= readedBytes;
    }
    return result;
}

int printErrorAndExit(char* errorText){
    perror(errorText);
    exit(1);
}

int readMessage(int fromSock, char* buffer){
    bzero(buffer, sizeof(buffer));
    int tmp;
    int size;
    tmp = readN(fromSock, &size, sizeof(int));

    if (tmp < 0) {
        perror("ERROR reading from socket");
        return tmp;
    }

    tmp = readN(fromSock, buffer, size);

    if (tmp < 0) {
        perror("ERROR reading from socket");
        return tmp;
    }

    return tmp;
}

int sendAll(int id, char* name, char* buffer){
    char* formedMessage[300] = {0};
    sprintf(formedMessage, "<%s> : %s", name, buffer);
    for (int i = 0; i < nClients; ++i)
    {   
        if (i == id)
        {
            continue;
        }
        sendContent(clients[i].sock, formedMessage);
    }
}

int sendContent(int destination, char* content){
    int check;
    printf("Отправляю длину сообщения\n");
    int size = strlen(content);
    check = write(destination, &size, sizeof(int));
    if (check <= 0)
    {
        printf("Ошибка передачи\n");
    }
    printf("Отправляю сообщение\n");
    check = write(destination, content, size);
    if (check <= 0)
    {
        printf("Ошибка передачи\n");
    }
}

void* handleClient(void* args){
    int id = *((int*) args);
    int destSock = clients[id].sock;

    char name[25];
    readMessage(destSock, name);

    pthread_mutex_lock(&mutex);
    clients[id].name = name;
    pthread_mutex_unlock(&mutex);

    char buffer[256];

    for (;;){
        if (readMessage(destSock, buffer) <= 0){
            break;
        }
        sendAll(id, name, buffer);
        printf("<%s> %s\n", name, buffer);
        fflush(stdout);
    }
}

char *get_time() {
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
