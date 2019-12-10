/**==================================================================================================================**
 | @name Sergeev A.A.                                                                                                 |
 | @group #3530901/60201                                                                                              |
 | @task #1-TCP                                                                                                       |
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
#include <pthread.h>
#include <thread>
#include <time.h>

using namespace std;

#define true  1
#define false 0
#define NUMBER_OF_CLIENTS 1000

/**********************************************************************************************************************
 *                                                  Global variables.                                                 *
 **********************************************************************************************************************/


int socket_descriptor;
map<int, string> clients;
list<thread> threads_list;
struct sockaddr_in server_address;

pthread_mutex_t cs_mutex;
int port;

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

void communicating(int new_socket_descriptor, string nickname);

void accept_connection() {
    struct sockaddr_in client_address;
    unsigned int client_length = sizeof(client_address);
    int nsd = accept(socket_descriptor, (struct sockaddr *) &client_address, &client_length);
    if (nsd <= 0) {
        perror("ERROR opening socket");
        return;
    }
    int nick_length;
    int temp = read(nsd, &nick_length, sizeof(int));
    if (temp < 0) {
        perror("ERROR reading from socket");
        return;
    }
    auto nick = new char[nick_length]();
    temp = read(nsd, nick, nick_length);
    if (temp < 0) {
        perror("ERROR reading from socket");
        return;
    }
    pthread_mutex_lock( &cs_mutex );
    clients[nsd] = string(nick);
    pthread_mutex_unlock( &cs_mutex );
    threads_list.emplace_back([&] { communicating(nsd, clients[nsd]); });
    free(nick);
}

/**********************************************************************************************************************
 *                                                IO-threads functions.                                               *
 **********************************************************************************************************************/


void communicating(int new_socket_descriptor, string nickname) {
    while (true) {
            int message_size;
            int temp = read(new_socket_descriptor, &message_size, sizeof(int));
            if (temp <= 0) {
                perror("ERROR reading from socket");
                pthread_mutex_lock( &cs_mutex );
                clients.erase(clients.find(new_socket_descriptor));
                pthread_mutex_unlock( &cs_mutex );
                close(new_socket_descriptor);
                break;
            }
            auto buffer = new char[message_size];
            temp = read(new_socket_descriptor, buffer, message_size);
            if (temp <= 0) {
                perror("ERROR reading from socket");
                pthread_mutex_lock( &cs_mutex );
                clients.erase(clients.find(new_socket_descriptor));
                pthread_mutex_unlock( &cs_mutex );
                close(new_socket_descriptor);
                break;
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
                        temp = write(it->first, &message_size, sizeof(int));
                        if (temp <= 0) {
                            perror("ERROR writing in socket");
                            continue;
                        }
                        temp = write(it->first, full_buffer, message_size);
                        if (temp <= 0) {
                            perror("ERROR writing in socket");
                            continue;
                        }
                    }
                }
            }
            free(buffer);
            free(full_buffer);
    }
    pthread_exit(0);
}

/**********************************************************************************************************************
 *                                                    main-function.                                                  *
 **********************************************************************************************************************/


int main(int argc, char *argv[]) {
    check_argc(argc, argv);
    initialization_socket_descriptor();
    initialization_socket_structure();
    bind_host_address();
//    listen(socket_descriptor, NUMBER_OF_CLIENTS);
    while (true) {
        listen(socket_descriptor, 1);
        accept_connection();
    }
    for (thread &t: threads_list) {
        t.join();
    }
}