//
// Created by Malik Hiraev on 03.10.2019.
//

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "../common/common.h"

#define BACKLOG 10

/**
 * Протокол
 *
 * 4 байта размер сообщения, дальше само сообщение
 *
 */

typedef struct Client {
    int fd;
    struct sockaddr_in *sockaddr;
    bool is_alive;
    char *name; // имя клиента. Максимум 32 символа
    struct Client *prev_client; // Предыдущий клиента
    struct Client *next_client; // Следующий клиент
} Client;

typedef struct Message {
    char *name; // Имя клиента
    char *msg; // ссылка на сообщеньку
    u_int32_t size; // размер сообщения
} Message;

int num_of_clients = 0;
char *divider = " : ";

void send_message_to_all_clients(Message *message);

int create_server(uint16_t port);

void clients_reader(void *args);

void listen_client(int sock_fd);

void remove_client(Client *dead_client);

void accept_client_to_list(Client *new_client);

void client_connected(Client *client);

void client_disconnected(Client *client);

void read_message(Client *client);

pthread_mutex_t lock; // ключ на работу с списком клиентов, при отключении, клиент будет удален.
Client *current_client = NULL;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        exit(0);
    }
    uint16_t portno = (uint16_t) atoi(argv[1]);
    // Initialize server
    int sock_fd = create_server(portno);
    printf("Server started at port: %i\n", portno);
    //Init mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("ERROR creating mutex for clients list");
        exit(1);
    }

    listen(sock_fd, BACKLOG);
    //Clients acceptor
    while (true) {
        listen_client(sock_fd);
    }
}

// Создает сервер на заданном порту, возвращает дескриптор
int create_server(uint16_t port) {
    struct sockaddr_in serv_addr;
    /* First call to socket() function */
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    int val = true; // Unnecessary variable, using because of function semantic requires it
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    /* Now bind the host address using bind() call.*/
    if (bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }
    return sock_fd;
}

// blocking function to get client socket
void listen_client(int sock_fd) {
    struct sockaddr_in cli_addr;
    unsigned client_len = sizeof(cli_addr);
    int new_sock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &client_len);
    if (new_sock_fd < 0) {
        perror("ERROR on accept");
        exit(1);
    }
    Client *new_client = (struct Client *) malloc(sizeof(struct Client)); // Выделяем память для хранения одного клиента

    pthread_t thread_id;
    // Инициализируем выделенную память новым клиентом.
    *new_client = (struct Client) {new_sock_fd, &cli_addr, true, NULL, NULL, NULL};
    accept_client_to_list(new_client); // Включаем нового клиента в текущий список клиентов
    // Выделение потока для работы с клиентом
    pthread_create(&thread_id, NULL, (void *) &clients_reader, new_client);
}

// Обработчик клиента в отдельном потоке (Чтение сообщений от клиента)
void clients_reader(void *args) {
    Client *client = (Client *) args;
    ssize_t n; //read value
    // firstly read user name
    char client_name[USER_NAME_SIZE]; // локальная переменная, можно не очищать для нее память, пока кдиент есть, есть и его имя
    n = readn(client->fd, &client_name[0], USER_NAME_SIZE);
    client->name = &client_name[0];
    if (n > 0) {
        client_connected(client);
        // Получили имя клиента
        while (client->is_alive) {
            read_message(client);
        }
        free(client); // Очищаем память
    } else {
        remove_client(client);
        free(client);
    }
}

// Присоединить клиента к списку всех клиентов.
void accept_client_to_list(Client *new_client) {
    pthread_mutex_lock(&lock);
    num_of_clients++;
    // Accepting
    if (current_client == NULL) {
        current_client = new_client;
    } else {
        current_client->next_client = new_client;
        new_client->prev_client = current_client;
        current_client = new_client;
    }
    pthread_mutex_unlock(&lock);
}

// Удалить из списка отключеннного клиента
void remove_client(Client *dead_client) {
    pthread_mutex_lock(&lock);
    if (current_client == dead_client) {
        current_client = current_client->prev_client;
    }
    if (dead_client->prev_client != NULL) {
        dead_client->prev_client->next_client = dead_client->next_client;
    }
    if (dead_client->next_client != NULL) {
        dead_client->next_client->prev_client = dead_client->prev_client;
    }
    dead_client->is_alive = false;
    close(dead_client->fd);
    num_of_clients--;
    pthread_mutex_unlock(&lock);
}

void print_client_name_and_address(Client *client) {
    __uint32_t cli_addr = client->sockaddr->sin_addr.s_addr;
    printf("%s (%d-%d.%d.%d.%d)", client->name, client->fd, (u_char) cli_addr, (u_char) (cli_addr >> 8),
           (u_char) (cli_addr >> 16), (u_char) (cli_addr >> 24));
}

void client_disconnected(Client *client) {
    print_client_name_and_address(client);
    printf(" disconnected\n");
    remove_client(client);
}

// Сообщить всем, что подключился новый клиент;
void client_connected(Client *client) {
    print_client_name_and_address(client);
    printf(" connected\n");
}

void read_message(Client *client) {
    ssize_t n;
    size_t msg_size = 0;
    size_t divider_len = strlen(divider);
    n = readn(client->fd, (char *) &msg_size, MSG_SIZE_VAL);
    if (n <= 0) { // Клиент умер/отключился
        client_disconnected(client);
    } else {
        char *msg = malloc(sizeof(char) * (msg_size + strlen(client->name) + divider_len));
        // В сообщение закладываем имя клиента и двоеточие
        strcpy(msg, client->name);
        strcat(msg, divider);
        Message *message = (struct Message *) malloc(sizeof(struct Message)); // Выделяем память для хранения сообщения
        *message = (struct Message) {client->name, &msg[0], (uint32_t) (msg_size + strlen(client->name) + divider_len)};
        //Читаем то кол-во сиволов, которое указано в заголовке сообщения
        // Записываем после символа :
        readn(client->fd, &msg[strlen(client->name) + divider_len], msg_size);
        printf("Got msg from %s\n", msg);
        send_message_to_all_clients(message);
    }
}

void send_message_to_all_clients(Message *message) {
    pthread_mutex_lock(&lock);
    Client *next_consumer = current_client;
    // Отправляем сообщения клиентам в обратном порядке
    while (next_consumer != NULL) {
        if (next_consumer->name != message->name) {
            write(next_consumer->fd, &(message->size), MSG_SIZE_VAL); // Пишем сначала размер сообщения
            write(next_consumer->fd, message->msg, message->size);
        }
        next_consumer = next_consumer->prev_client;
    }
    pthread_mutex_unlock(&lock);
    free(message->msg);
    free(message);
}
