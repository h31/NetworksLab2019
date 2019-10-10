//
// Created by danila on 10/10/2019.
//

struct Client {
    int sockfd;
    char *name;
    int status;
    pthread_t thread;

} typedef Client;

struct Message {
    char *buffer;
    int size;
} typedef Message;
