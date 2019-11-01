#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <fcntl.h>
#include <bits/fcntl-linux.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>

#include "../header.h"

#define PORT 5001;

void write_msg_to_client(int fd, char *buffer, uint32_t buff_size);

uint32_t get_size_of_msg(int fd);

char *read_msg(int fd, char *msg, char *buffer, uint32_t buff_size);

void client_exit(ClientChain *client_data);

void init_socket();

void init_server_pollfd();

void bind_listen();

void delete_pollfd(int fd);

void exit_server();

void increase_fds();

void decrease_fds();

ClientChain *root, *last;
int sockfd, poll_size;
struct pollfd *fds;

int main(int argc, char *argv[]) {
    unsigned int clilen;
    struct sockaddr_in cli_addr;

    int temp_size;
    int check;
    char *buffer, *msg;
    uint32_t buff_size;
    poll_size = 1;

    if (signal(SIGINT, exit_server) == SIG_ERR) {
        perror("ERROR on sigint_handler");
        exit(1);
    }

    init_socket();

    bind_listen();

    clilen = sizeof(cli_addr);
    root = client_init(sockfd);
    last = root;

    fds = (struct pollfd *) malloc(sizeof(struct pollfd));

    init_server_pollfd();

    while (1) {
        check = poll(fds, poll_size, -1);
        if (check < 0) {
            perror("ERROR on poll");
            break;
        }

        temp_size = poll_size;
        for (int i = 0; i < temp_size; i++) {

            if (fds[i].revents == 0) {
                continue;
            }

            if (fds[i].revents != POLLIN) {
                printf("ERROR wrong revents = %d\n", fds[i].revents);
                exit_server();
            }

            if (fds[i].fd == sockfd) {
                //accept client
                while (1) {
                    check = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                    if (check < 0) {
                        break;
                    }

                    increase_fds();
                    fds[poll_size - 1].fd = check;
                    fds[poll_size - 1].events = POLLIN;

                    ClientChain *client = client_init(check);
                    last->next = client;
                    last = client;

                }
            } else {
                //read message
                buff_size = get_size_of_msg(fds[i].fd);
                buffer = (char *) malloc(buff_size);
                msg = (char *) malloc(buff_size + 25);

                int temp_fd = fds[i].fd;

                strcpy(msg, read_msg(fds[i].fd, msg, buffer, buff_size));

                for (int j = 0; j < poll_size; j++) {
                    if (fds[j].fd != sockfd && fds[j].fd != temp_fd) {
                        write_msg_to_client(fds[j].fd, msg, buff_size + 25);
                    }
                }
                free(buffer);
                free(msg);
            }
        }
    }
    exit_server();
    return 0;
}

void init_socket() {
    int on = 1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit_server();
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0) {
        perror("ERROR on setsockopt");
        exit_server();
    }

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        perror("ERROR making socket nonblock");
        exit_server();
    }
}

void init_server_pollfd() {
    bzero(fds, sizeof(fds));
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
}

void bind_listen() {
    uint16_t portno;
    struct sockaddr_in serv_addr;

    portno = PORT

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit_server();
    }

    if (listen(sockfd, 5) < 0) {
        perror("ERROR on listen");
        exit_server();
    }
}

void write_msg_to_client(int fd, char *buffer, uint32_t buff_size) {
    if (write(fd, &buff_size, sizeof(int)) < 0) {
        perror("ERROR writing from socket");
        exit_server();
    }
    if (write(fd, buffer, buff_size) < 0) {
        perror("ERROR writing from socket");
        exit_server();
    }
}

uint32_t get_size_of_msg(int fd) {
    uint32_t buff_size;
    if (read(fd, &buff_size, sizeof(int)) < 0) {
        if (errno != EWOULDBLOCK) {
            perror("ERROR reading from socket");
            exit_server();
        }
    }
    return buff_size;
}

char *read_msg(int fd, char *msg, char *buffer, uint32_t buff_size) {
    int check;
    check = readn(fd, buffer, buff_size);
    if (check < 0) {
        if (errno != EWOULDBLOCK) {
            perror("ERROR reading from socket");
            exit_server();
        }
    }

    ClientChain *client = get_client(fd, root);

    if (check == 0 || !strcmp(buffer, "/exit")) {
        printf("Client \"%s\" disconnected\n", client->name);
        sprintf(msg, "Client \"%s\" disconnected\n", client->name);
        delete_pollfd(client->fd);
        client_exit(get_client(fd, root));
    } else if (!client->flag) {
        client->name = (char *) malloc(buff_size);
        strcpy(client->name, buffer);
        printf("Client \"%s\" connected\n", buffer);
        sprintf(msg, "Client \"%s\" connected\n", buffer);
        client->flag = 1;
    } else {
        printf("message from %s: %s\n", client->name, buffer);
        sprintf(msg, "<%s> %s: %s\n", get_time(), client->name, buffer);
    }

    return msg;
}

void client_exit(ClientChain *client_data) {
    close(client_data->fd);
    ClientChain *temp = root;
    while (temp->next != client_data) {
        temp = temp->next;
    }
    if (client_data->next == NULL && root->next == client_data) {
        close(sockfd);
        printf("All users exit chat.\n");
        root->next = NULL;
        last = root;
        free(client_data);
        init_socket();
        bind_listen();
        init_server_pollfd();
    } else if (client_data->next == NULL) {
        last = temp;
        temp->next = NULL;
        free(client_data);
    } else {
        temp->next = client_data->next;
        free(client_data);
    }
}

void delete_pollfd(int fd) {
    for (int i = 0; i < poll_size; i++) {
        if (fds[i].fd == fd) {
            for (int j = i; j < poll_size; j++) {
                if (j != poll_size - 1) {
                    fds[j] = fds[j + 1];
                }
            }
            decrease_fds();
            break;
        }
    }
}

void increase_fds() {
    poll_size++;
    fds = (struct pollfd *) realloc(fds, poll_size * sizeof(struct pollfd));
}

void decrease_fds() {
    poll_size--;
    fds = (struct pollfd *) realloc(fds, poll_size * sizeof(struct pollfd));
}

void exit_server() {
    while (root->next != NULL) {
        client_exit(root->next);
    }
    close(sockfd);
    exit(EXIT_SUCCESS);
}


