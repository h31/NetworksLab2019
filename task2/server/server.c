#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

#include "../common/common.h"
#include "client.h"
#include "clients_handler.h"
#include "printer.h"
#include "fds_worker.h"

#define BACKLOG 10

static char *divider = " : ";

static int sock_fd;

static int create_server(uint16_t port) {
    struct sockaddr_in serv_addr;
    int rc, val = true;
    /* First call to socket() function */
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    rc = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (rc < 0) {
        perror("setsockopt() failed");
        close(sock_fd);
        close(sock_fd);
        exit(-1);
    }
    // Make socket non-blocking
    rc = ioctl(sock_fd, FIONBIO, (char *) &val);
    if (rc < 0) {
        perror("ioctl() failed");
        close(sock_fd);
        exit(-1);
    }
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

static int end_server() {
    close_all_fds();
    exit(1);
}

static Message *build_message_with_username(Client *sender) {
    Message *message = sender->message;
    u_int32_t full_msg_size = (u_int32_t) (message->size + strlen(sender->name) + strlen(divider));
    char *msg = malloc(sizeof(char) * full_msg_size);
    // В сообщение закладываем имя клиента и двоеточие
    strcpy(msg, sender->name);
    strcat(msg, divider);
    strcat(msg, message->msg);
    Message *message_with_username = (Message *) malloc(sizeof(Message));
    *message_with_username = (Message) {msg, full_msg_size};
    return message_with_username;
}

static void send_message_to_all_clients(Client *sender) {
    Message *message = build_message_with_username(sender);
    Client *next_consumer = get_first_client();
    // Отправляем сообщения клиентам в обратном порядке
    while (next_consumer != NULL) {
        if (next_consumer != sender) {
            int fd = get_pollfd(next_consumer->fd_index)->fd;
            write(fd, &(message->size), MSG_SIZE_VAL); // Пишем сначала размер сообщения
            write(fd, message->msg, message->size);
        }
        next_consumer = next_consumer->next_client;
    }
    free(sender->message->msg);
    free(message->msg);
    free(message);
    sender->message->msg = NULL;
    sender->message->size = 0;
}

static bool read_client_name(Client *client) {
    size_t offset = USER_NAME_SIZE - client->waiting_bytes;
    ssize_t r = read(get_pollfd(client->fd_index)->fd, &client->name[offset], client->waiting_bytes);
    if (r == 0) {
        return false;
    }
    if (r < client->waiting_bytes) {
        client->waiting_bytes -= r;
    } else {
        client->waiting_bytes = 0;
    }
    return true;
}

static bool read_msg_size(Client *client) {
    size_t offset = MSG_SIZE_VAL - client->waiting_bytes;
    size_t msg_size = 0;
    ssize_t r = read(get_pollfd(client->fd_index)->fd, (char *) &msg_size, client->waiting_bytes);
    msg_size = client->message->size + (msg_size << (8 * offset));
    if (r == 0) {
        return false;
    }
    if (r < client->waiting_bytes) {
        client->waiting_bytes -= r;
    } else {
        client->message->msg = malloc(sizeof(char) * msg_size);
        client->waiting_bytes = 0;
    }
    client->message->size = msg_size;
    return true;
}

static bool read_client_msg(Client *client) {
    size_t offset = client->message->size - client->waiting_bytes;
    ssize_t r = read(
            get_pollfd(client->fd_index)->fd,
            &client->message->msg[offset],
            client->waiting_bytes
    );
    if (r == 0) {
        return false;
    }
    if (r < client->waiting_bytes) {
        client->waiting_bytes -= r;
    } else {
        client->waiting_bytes = 0;
    }
    return true;
}

/**
 * If client has connected status, we have to read and initialize his name
 * if client has ready status we have to read first 4 bytes to know the size of massage and
 * than read massage with known size
 * @param client
 */
static void read_message(Client *client) {
    bool r;
    if (client->status == WAIT_FOR_NAME) {
        r = read_client_name(client);
    } else if (client->status == WAIT_FOR_SIZE) {
        r = read_msg_size(client);
    } else {
        r = read_client_msg(client);
    }
    if (r) {
        if (client->waiting_bytes == 0) {
            switch (client->status) {
                case WAIT_FOR_NAME:
                    client->status = WAIT_FOR_SIZE;
                    client->waiting_bytes = MSG_SIZE_VAL;
                    break;
                case WAIT_FOR_SIZE:
                    client->status = WAIT_FOR_MSG;
                    client->waiting_bytes = client->message->size;
                    break;
                case WAIT_FOR_MSG:
                    client->status = WAIT_FOR_SIZE;
                    client->waiting_bytes = MSG_SIZE_VAL;
                    send_message_to_all_clients(client);
            }
        }
    } else {
        print_client_disconnected(client);
        remove_and_close_fd(client->fd_index);
        remove_client(client);
    }
}

// sock_fd is a constant, it doesn't change
static void handle_new_client() {
    if (get_num_of_clients() == MAX_CLIENTS_SIZE) return;
    struct sockaddr_in cli_addr;
    unsigned client_len = sizeof(cli_addr);
    int new_sock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &client_len);
    if (new_sock_fd < 0) {
        perror("ERROR on accept");
        end_server();
    }
    int fd_index = add_fd(new_sock_fd);
    /**
     * If file descriptor cannot be added to fds_list, because of there are no any space,
     * add_function returns -1 so we can't communicate with this client
     */
    if (fd_index == -1) return;
    Client *new_client = (Client *) malloc(sizeof(Client));
    char *name = (char *) malloc(sizeof(char) * USER_NAME_SIZE);
    Message *message = (Message *) malloc(sizeof(Message));
    *message = (Message) {NULL, 0};
    // Init with USER_NAME_SIZE to wait
    *new_client = (Client) {fd_index, &cli_addr, WAIT_FOR_NAME, USER_NAME_SIZE, message, name, NULL, NULL};
    print_client_connected(new_client);
    accept_client(new_client);
}

static void handle_readable_descriptors() {
    // Check if new client is accepted
    int acceptor_revents = get_acceptor_revents();
    if (acceptor_revents == POLLIN) {
        handle_new_client();
    } else if (acceptor_revents != 0) {
        perror("Unexpected revents value for acceptor");
        end_server();
    }
    Client *next_client = get_first_client();
    while (next_client != NULL) {
        struct pollfd *poll_fd = get_pollfd(next_client->fd_index);
        if (poll_fd->revents == POLLIN) {
            read_message(next_client);
        } else if (poll_fd->revents != 0) {
            print_client_disconnected(next_client);
            remove_and_close_fd(next_client->fd_index);
            remove_client(next_client);
        }
        poll_fd->revents = 0;
        next_client = next_client->next_client;
    }
}

static void listen_poll() {
    int rc = poll_inner();
    if (rc < 0) {
        perror("poll failed");
        end_server();
    } else {
        handle_readable_descriptors();
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        exit(0);
    }
    uint16_t portno = (uint16_t) atoi(argv[1]);
    // Initialize server
    sock_fd = create_server(portno);
    printf("Server started at port: %i\n", portno);

    listen(sock_fd, BACKLOG);
    init_fds_worker(sock_fd);
    //Clients acceptor
    while (true) {
        listen_poll();
    }
}
