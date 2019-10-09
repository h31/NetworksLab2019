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

int receive_int (int *num, int fd);
void* handleClient(void* args);
int readMessage(int fromSock, char* buffer);
int readN(int socket, char* buf, int length);
int sendAll(int id, char* name, char* buffer);
int sendContent(int destination, char* content);

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;
    int word_length_buffer;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    int newsockfd;
    int client_id;

    for (;;){
        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

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
