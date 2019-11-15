//
// Created by Malik Hiraev on 15.11.2019.
//

#ifndef NETWORKSLAB2019_CLIENT_H
#define NETWORKSLAB2019_CLIENT_H

#include "message.h"

#define MAX_CLIENTS_SIZE 1024

enum client_status {
    WAIT_FOR_NAME, WAIT_FOR_SIZE, WAIT_FOR_MSG
};

typedef struct Client {
    int fd_index;
    struct sockaddr_in *sockaddr;
    enum client_status status;
    Message *message;
    char *name; // имя клиента. Максимум 32 символа
    struct Client *prev_client; // Предыдущий клиента
    struct Client *next_client; // Следующий клиент
} Client;

#endif //NETWORKSLAB2019_CLIENT_H
