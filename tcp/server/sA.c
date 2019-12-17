#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include <string.h>
#include "clList.h"

#define BACKLOG 5
#define MAXL_NAME 20

void *client_handler(void *args);

void message_handler(client *cl);

client *list = NULL;
pthread_mutex_t mutex;

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    char buffer[256];
    struct sockaddr_in serv_addr;
    ssize_t n;
    struct sockaddr_in cli_addr;
    unsigned int clilen;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    //https://stackoverflow.com/questions/24194961/how-do-i-use-setsockoptso-reuseaddr
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

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
    listen(sockfd, BACKLOG);
    mutex = get_mutex();

    //инициализируем мьютекс для работы с листом
    if (pthread_mutex_init(&mutex, NULL) < 0) {
        perror("ERROR on creating mutex");
        exit(1);
    }
    set_mutex(mutex);

    clilen = sizeof(cli_addr);

/* Accept actual connection from the client */
    for (;;) {

        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        client *new_cl = (struct client *) malloc(sizeof(struct client));

//поток под клиентское подключение (включая ошибки) 
        pthread_t thrd;
        *new_cl = (struct client) {newsockfd, NULL, &cli_addr, true, NULL, NULL};
        push_el(new_cl);
        list = get_header();
        int res = pthread_create(&thrd, NULL, client_handler, (void *) (new_cl));
        if (res) {
            printf("Error while creating new thread\n");
            exit(1);
        }

    }
    close(sockfd);
    mutex = get_mutex();
    if (pthread_mutex_destroy(&mutex) < 0)
        perror("ERROR on destroying mutex");
    exit(1);
    return 0;
}


void *client_handler(void *args) {
    client *cl = (client *) args;
    char name[MAXL_NAME];

    uint32_t reclen;
    read(cl->socket, (char *) &reclen, sizeof(reclen));

    int res = readn(cl->socket, &name[0], reclen);
    if (res > 0) {
        cl->name = &name[0];
        printf("%s joined the chat\n", &name[0]);
        fflush(stdout);
        while (cl->state) {
            fflush(stdout);
            message_handler(cl);
            fflush(stdout);
        }
        printf("%s left the chat\n", &name[0]);
        free(cl);
    } else {
        remove_cl(cl);
        free(cl);
    }
}

//Snader Effective TCP programming listing 2.13
void message_handler(client *cl) {
    uint32_t reclen;
    int rc;
    fflush(stdout);
    rc = read(cl->socket, (char *) &reclen, sizeof(reclen));
    if (rc <= 0) {
        printf("%s is not available \n", cl->name);
        cl->state = false;
    } else {
        char *text = malloc(sizeof(char) * (3 + strlen(cl->name) + reclen + 1));
        //формируем сообщение
        strcpy(text, "<");
        strcat(text, cl->name);
        strcat(text, "> ");
        packet *p = (struct packet *) malloc(sizeof(struct packet));
        *p = (struct packet) {(uint32_t)(2 + strlen(cl->name) + reclen), cl->name, &text[0]};
        readn(cl->socket, &text[3 + strlen(cl->name)], reclen);
        printf(" New message %s\n", text);
        //отправка всем
        broadcast_to_list(p);
    }
    return;
}




