#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>

#include "../header.h"
#include "../params.h"

void communicate_to_client(void *arg);

void client_exit(ClientChain *client_data);

void send_msg_to_clients(ClientChain *sender_data, char msg[], int buff_size);

char *get_time();

ClientChain *root, *last;
int sockfd;

int main(int argc, char *argv[]) {
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);


    root = chain_init(sockfd);
    last = root;

    pthread_t tid;
    while (1) {
        ClientChain *this = chain_init(sockfd);
        this->sock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (this->sock < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        this->prev = last;
        last->next = this;
        last = this;

        if (pthread_create(&tid, NULL, (void *) communicate_to_client, (void *) this) != 0) {
            printf("thread has not created");
            exit(1);
        }
    }
    return 0;
}

void communicate_to_client(void *arg) {
    ClientChain *client_data = (ClientChain *) arg;
    char *buffer;
    char *msg;
    int buff_size;
    char name[NAME_LEN];

    //get client name
    bzero(name, NAME_LEN);
    if (readn(client_data->sock, name, NAME_LEN) < 0) {
        perror("ERROR reading from socket");
        exit(1);
    } else {
        make_str(name);
        strncpy(client_data->name, name, NAME_LEN);
        printf("<%s>---%s connected---\n", get_time(), name);
        msg = (char *) malloc(25);
        sprintf(msg, "<%s>---%s connected---\n", get_time(), name);
        send_msg_to_clients(client_data, msg, 25);
    }

    //main cycle to get message from client and send it to other clients
    while (1) {
        //get message size
        buff_size = 0;
        if (read(client_data->sock, &buff_size, sizeof(int)) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }

        //get message
        buffer = (char *) malloc(buff_size);
        if (readn(client_data->sock, buffer, buff_size) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        make_str(buffer);

        //check if clint exit
        if (strcmp(buffer, "/exit") == 0) {
            printf("<%s>---%s exit chat---\n", get_time(), name);
            msg = (char *) malloc(50);
            sprintf(msg, "<%s>---%s exit chat---\n", get_time(), name);
            send_msg_to_clients(client_data, msg, 50);
            client_exit(client_data);
            break;
        } else {
            int msg_size = buff_size + 50;
            printf("<%s>%s: %s\n", get_time(), name, buffer);
            msg = (char *) malloc(msg_size);
            sprintf(msg, "<%s>%s: %s\n", get_time(), name, buffer);
            send_msg_to_clients(client_data, msg, msg_size);
        }
    }
}


void send_msg_to_clients(ClientChain *sender_data, char msg[], int buff_size) {
    ClientChain *temp = root->next;
    while (temp != NULL) {
        if (temp != sender_data) {
            //send message size
            if (write(temp->sock, &buff_size, sizeof(int)) < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            //send message
            if (write(temp->sock, msg, buff_size) < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
        }
        temp = temp->next;
    }
}

void client_exit(ClientChain *client_data) {
    close(client_data->sock);
    if (client_data->next == NULL && client_data->prev == root) {
        close(sockfd);
        printf("All users exit chat, see u later\n");
        exit(EXIT_SUCCESS);
    } else if (client_data->next == NULL) {
        last = client_data->prev;
        client_data->prev->next = NULL;
        free(client_data);
        pthread_exit(NULL);
    } else {
        client_data->prev->next = client_data->next;
        client_data->next->prev = client_data->prev;
        free(client_data);
        pthread_exit(NULL);
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

