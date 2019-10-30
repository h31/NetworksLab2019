#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>

#include "../header.h"

void communicate_to_client(void *arg);

void client_exit(ClientChain *client_data);

void send_msg_to_clients(ClientChain *sender_data, char msg[], int buff_size);

ClientChain *root, *last;
int sockfd;
pthread_mutex_t mtx;

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

    if (pthread_mutex_init(&mtx, NULL) != 0) {
        perror("ERROR creating mutex");
        exit(1);
    }

    while (1) {
        ClientChain *this = chain_init(sockfd);
        this->sock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (this->sock < 0) {
            perror("ERROR on accept");
            exit(1);
        }

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
    uint32_t buff_size, msg_size, name_size;
    
    //get name size
    name_size = 0;
    int res;
    if ((res = read(client_data->sock, &name_size, sizeof(int))) < 0) {
        perror("ERROR reading from socket");
        exit(1);
    } else if (res == 0) {
        client_exit(client_data);
    }

    //get name
    client_data->name = (char *) malloc(name_size);
    buffer = (char *) malloc(name_size);
    if ((res = readn(client_data->sock, buffer, name_size)) < 0) {
        perror("ERROR reading from socket");
        exit(1);
    } else if(res == 0) {
        client_exit(client_data);
    } else {
        msg_size = name_size + 30;
        make_str_without_line_break(buffer);
        strncpy(client_data->name, buffer, name_size);
        printf("<%s>---%s connected---\n", get_time(), buffer);
        msg = (char *) malloc(msg_size);
        sprintf(msg, "<%s>---%s connected---\n", get_time(), buffer);
        send_msg_to_clients(client_data, msg, msg_size);
        free(msg);
    }

    //main cycle to get message from client and send it to other clients
    while (1) {
        //get message size
        buff_size = 0;
        int res;
        if ((res = read(client_data->sock, &buff_size, sizeof(int))) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        } else if (res == 0) {
            client_exit(client_data);
        }

        //get message
        buffer = (char *) malloc(buff_size);
        if ((res = readn(client_data->sock, buffer, buff_size)) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        } else if(res == 0) {
            client_exit(client_data);
        }
        make_str_without_line_break(buffer);

        //check if clint exit
        if (strcmp(buffer, "/exit") == 0) {
            msg_size = sizeof(client_data->name) + 50;
            printf("<%s>---%s exit chat---\n", get_time(), client_data->name);
            msg = (char *) malloc(msg_size);
            sprintf(msg, "<%s>---%s exit chat---\n", get_time(), client_data->name);
            send_msg_to_clients(client_data, msg, msg_size);
            free(msg);
            client_exit(client_data);
            break;
        } else {
            msg_size = buff_size + 50;
            printf("<%s>%s: %s\n", get_time(), client_data->name, buffer);
            msg = (char *) malloc(msg_size);
            sprintf(msg, "<%s>%s: %s\n", get_time(), client_data->name, buffer);
            send_msg_to_clients(client_data, msg, msg_size);
            free(msg);
        }
    }
}


void send_msg_to_clients(ClientChain *sender_data, char msg[], int buff_size) {
    ClientChain *temp = root->next;
    pthread_mutex_lock(&mtx);
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
    pthread_mutex_unlock(&mtx);
}

void client_exit(ClientChain *client_data) {
    close(client_data->sock);
    ClientChain *temp = root;
    while (temp->next != client_data) {
        temp = temp->next;
    }
    if (client_data->next == NULL && root->next == client_data) {
        close(sockfd);
        printf("All users exit chat, see u later\n");
        exit(EXIT_SUCCESS);
    } else if (client_data->next == NULL) {
        last = temp;
        temp->next = NULL;
        free(client_data);
        pthread_exit(NULL);
    } else {
        temp->next = client_data->next;
        free(client_data);
        pthread_exit(NULL);
    }
}



