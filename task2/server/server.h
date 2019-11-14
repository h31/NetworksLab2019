//
// Created by Malik Hiraev on 04.11.2019.
//

#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

#include "../common/common.h"
#include "clients_handler.h"
#include "printer.h"
#include "fds_worker.h"

#define BACKLOG 10
#define MAX_CLIENTS_SIZE 1024

enum client_status {
    WAIT_FOR_NAME, WAIT_FOR_SIZE, WAIT_FOR_MSG
};

typedef struct Message {
    char *msg; // ссылка на сообщеньку
    size_t size; // размер сообщения
} Message;

typedef struct Client {
    int fd_index;
    struct sockaddr_in *sockaddr;
    enum client_status status;
    long last_bytes_get_time; //Время последнего получения новых байтов
    Message *message;
    char *name; // имя клиента. Максимум 32 символа
    struct Client *prev_client; // Предыдущий клиента
    struct Client *next_client; // Следующий клиент
} Client;

#endif //SERVER_H
