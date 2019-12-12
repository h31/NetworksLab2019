/**==================================================================================================================**
 | @name Sergeev A.A.                                                                                                 |
 | @group #3530901/60201                                                                                              |
 | @task #2-TCP (non blocking with poll-function)                                                                     |
 | @file server.cpp                                                                                                   |
 **==================================================================================================================**/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <list>
#include <map>
#include <string.h>
#include <time.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

using namespace std;

#define true  1
#define false 0

/**********************************************************************************************************************
 *                                                  Global variables.                                                 *
 **********************************************************************************************************************/


int socket_descriptor;
map<int, string> clients;
list<struct pollfd> clients_pollfd;
struct sockaddr_in server_address;
int port;
int new_socket_descriptor;

/**********************************************************************************************************************
 *                                                Auxiliary functions.                                                *
 **********************************************************************************************************************/

void check_argc(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage %s port\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);
}

/**********************************************************************************************************************
 *                                               TCP-connect functions.                                               *
 **********************************************************************************************************************/


void initialization_socket_descriptor() {
    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
}

void initialization_socket_structure() {
    uint16_t port_number = htons((uint16_t) port);

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = port_number;
}

void bind_host_address() {
    if (bind(socket_descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }
}

void accept_connection() {
    struct sockaddr_in client_address;
    unsigned int client_length = sizeof(client_address);
    int nsd = accept(socket_descriptor, (struct sockaddr *) &client_address, &client_length);
    if (nsd <= 0) {
        perror("ERROR opening socket");
        return;
    }
    int nick_length;
    int tempi = read(nsd, &nick_length, sizeof(int));
    if (tempi < 0) {
        perror("ERROR reading from socket");
        return;
    }
    auto nick = new char[nick_length]();
    tempi = read(nsd, nick, nick_length);
    if (tempi < 0) {
        perror("ERROR reading from socket");
        return;
    }
    clients[nsd] = string(nick);
    struct pollfd temp;
    temp.fd = nsd;
    temp.events = POLLRDNORM;
    clients_pollfd.push_back(temp);
    free(nick);
}

/**********************************************************************************************************************
 *                                                IO-threads functions.                                               *
 **********************************************************************************************************************/
int temp_nsd;

bool is_check(const struct pollfd &fd) {
    return fd.fd == new_socket_descriptor;
}

void communicating() {
    map<int, string>::iterator nick_it;
    nick_it = clients.find(new_socket_descriptor);
    string nickname = nick_it->second;
    int message_size;
    int tempi = read(new_socket_descriptor, &message_size, sizeof(int));
    if (tempi <= 0) {
        perror("ERROR reading from socket");
        clients.erase(clients.find(new_socket_descriptor));

        clients_pollfd.remove_if(is_check);
        close(new_socket_descriptor);
        return;
    }
    auto buffer = new char[message_size];
    tempi = read(new_socket_descriptor, buffer, message_size);
    if (tempi <= 0) {
        perror("ERROR reading from socket");
        clients.erase(clients.find(new_socket_descriptor));
        close(new_socket_descriptor);
        return;
    }
    message_size = message_size + nickname.length() + 6;
    auto full_buffer = new char[message_size];

    if (strcmp(buffer, "") != 0) {
        strcpy(full_buffer, "[");
        strcat(full_buffer, nickname.c_str());
        strcat(full_buffer, "] -> ");
        strcat(full_buffer, buffer);
        printf("%s\n", full_buffer);

        for (map<int, string>::iterator it = clients.begin(); it != clients.end(); it++) {
            if (it->first > 0) {
                tempi = write(it->first, &message_size, sizeof(int));
                if (tempi <= 0) {
                    perror("ERROR writing in socket");
                    continue;
                }
                tempi = write(it->first, full_buffer, message_size);
                if (tempi <= 0) {
                    perror("ERROR writing in socket");
                    continue;
                }
            }
        }
    }
    free(buffer);
    free(full_buffer);
}

/**********************************************************************************************************************
 *                                                    main-function.                                                  *
 **********************************************************************************************************************/


int main(int argc, char *argv[]) {
    check_argc(argc, argv);
    initialization_socket_descriptor();
    initialization_socket_structure();
    bind_host_address();

    struct pollfd temp;
    clients_pollfd.push_back(temp);
    clients_pollfd.back().fd = socket_descriptor;
    clients_pollfd.back().events = POLLRDNORM;

    listen(socket_descriptor, 1);

    while (true) {
        struct pollfd *arr = new struct pollfd[clients_pollfd.size()];
        copy(clients_pollfd.begin(), clients_pollfd.end(), arr);
        int nready = poll(arr, clients_pollfd.size(), -1);
        if (arr[0].revents && POLLRDNORM) { /* новое соединение с клиентом */
            accept_connection();
            listen(socket_descriptor, 1);
        }
        for (int i = 1; i < clients_pollfd.size(); i++) { /* проверяем все клиенты на наличие данных */
            if (arr[i].revents && (POLLRDNORM | POLLERR)) {
                new_socket_descriptor = arr[i].fd;
                communicating();
            }
        }
        free(arr);
    }
}