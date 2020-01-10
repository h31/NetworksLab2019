#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "signal.h"
#include <arpa/inet.h>

#include "../common/common.h"

#define PORT_NO 5001
#define MAX_CLIENTS 10

int sockfd;
time_t now;
struct tm time_struct;

pthread_mutex_t mutex;

typedef struct {
    int socket;
    char *name;
} client;

client *clients[MAX_CLIENTS];
int clients_count = 0;

void init_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }
}

void send_message_to_clients(int socket, char *message, char *name) {
    char *msg = (char *) malloc((strlen(message) + strlen(name)) * sizeof(char));
    strcat(msg, ANSI_COLOR_BLUE);
    strcat(msg, "[");
    strcat(msg, name);
    strcat(msg, "]");
    strcat(msg, ANSI_COLOR_RESET);
    strcat(msg, ": ");
    strcat(msg, message);
    int n;
    int len = strlen(msg);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            if (clients[i]->socket != socket) {
                n = write(clients[i]->socket, &len, sizeof(int));
                if (n <= 0) {
                    perror("ERROR writing length to socket");
                    exit(1);
                }
                n = write(clients[i]->socket, msg, len);
                if (n <= 0) {
                    perror("ERROR writing to socket");
                    exit(1);
                }
            }
        }
    }
}

void close_client(int socket) {
    pthread_mutex_lock(&mutex);
    shutdown(socket, SHUT_RDWR);
    close(socket);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            if (clients[i]->socket == socket) {
                printf(ANSI_COLOR_GREEN"<%02d:%02d> "ANSI_COLOR_BLUE"[%s]"ANSI_COLOR_RESET" disconnected\n",
                       time_struct.tm_hour,
                       time_struct.tm_min, clients[i]->name);
                clients[i] = NULL;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void signal_handler() {
    for (int i = 0; i < clients_count; i++) {
        close_client(clients[i]->socket);
    }
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    printf("Server stopped\n");
    exit(0);
}

void *client_func(void *new_cl) {
    client *client1 = *(client **) new_cl;
    char buffer[MESSAGE_LEN];
    int socket = client1->socket;
    char *name = client1->name;

    /* If connection is established then start communicating */
    while (1) {
        bzero(buffer, MESSAGE_LEN);
        int msg_size = 0;
        int status = read(socket, &msg_size, sizeof(int));
        if (status < 0) {
            perror("ERROR reading length from socket");
            exit(1);
        }
        if (msg_size != 0) {
            status = read(socket, buffer, msg_size);
            if (status < 0) {
                perror("ERROR reading from socket");
                exit(1);
            }
            time(&now);
            time_struct = *localtime(&now);
            printf(ANSI_COLOR_GREEN"<%02d:%02d> "ANSI_COLOR_BLUE"[%s]"ANSI_COLOR_RESET": %s", time_struct.tm_hour,
                   time_struct.tm_min, name, buffer);

            /* Write a responses to clients */
            send_message_to_clients(socket, buffer, name);
        } else {
            close_client(socket);
            clients_count--;
        }
    }
}

int main() {
    int newsockfd;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int status;

    init_clients();

    signal(SIGINT, signal_handler);
    pthread_mutex_init(&mutex, NULL);

    pthread_t client_thread_id;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT_NO);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }
    printf("Start Server on: %s:%d\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, MAX_CLIENTS);
    clilen = sizeof(cli_addr);

    while (1) {
        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }
        // add client
        client *new_client = (client *) malloc(sizeof(client));
        int username_len = 0;
        status = read(newsockfd, &username_len, sizeof(int));
        if (status < 0) {
            perror("ERROR reading username length from socket");
            exit(1);
        }
        // alloc (len + 1) - we need EOL symbol
        char *username = (char *) malloc(username_len + 1);
        bzero(username, username_len + 1);
        // read username
        status = read(newsockfd, username, username_len);
        if (status < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        time(&now);
        time_struct = *localtime(&now);
        printf(ANSI_COLOR_GREEN"<%02d:%02d> "ANSI_COLOR_BLUE"[%s]"ANSI_COLOR_RESET" connected\n", time_struct.tm_hour,
               time_struct.tm_min, username);
        new_client->socket = newsockfd;
        new_client->name = username;

        // add new client to an array
        for (int i = 0; i <= clients_count; i++) {
            if (clients[i] == NULL) {
                clients[i] = new_client;
                clients_count++;
                break;
            }
        }

        pthread_create(&client_thread_id, NULL, client_func, &new_client);
    }
}
