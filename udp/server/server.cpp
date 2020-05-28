/**==================================================================================================================**
 | @name Sergeev A.A.                                                                                                 |
 | @group #3530901/60201                                                                                              |
 | @task #3-UDP                                                                                                       |
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
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
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

/**********************************************************************************************************************
 *                                                IO-threads functions.                                               *
 **********************************************************************************************************************/


void communicating() {
    clients[socket_descriptor] = "";
    while (true) {
        int message_size = 1000;
        socklen_t size_addr = sizeof(server_address);
        int temp = recvfrom(socket_descriptor, message_size, sizeof(int), 0, (struct sockaddr *) &server_address, &size_addr);
        if (temp <= 0) {
            perror("ERROR reading from socket - 1");
            pthread_mutex_lock( &cs_mutex );
            clients.erase(clients.find(socket_descriptor));
            pthread_mutex_unlock( &cs_mutex );
            close(socket_descriptor);
            break;
        }
        auto buffer = new char[message_size];
        int temp = recvfrom(socket_descriptor,  buffer, message_size, 0, (struct sockaddr *) &server_address, &size_addr);
        if (temp <= 0) {
            perror("ERROR reading from socket - 2");
            pthread_mutex_lock( &cs_mutex );
            clients.erase(clients.find(socket_descriptor));
            pthread_mutex_unlock( &cs_mutex );
            close(socket_descriptor);
            break;
        }
        if (clients[socket_descriptor] == "") {
            clients[socket_descriptor] = string(buffer);
            continue;
        }
        string nickname = clients[socket_descriptor];
        message_size = message_size + nickname.length() + 6;
        auto full_buffer = new char[message_size];

        if (strcmp(buffer, "") != 0) {
            strcpy(full_buffer, "[");
            strcat(full_buffer, nickname.c_str());
            strcat(full_buffer, "] -> ");
            strcat(full_buffer, buffer);
            printf("%s\n", full_buffer);

            temp = sendto(socket_descriptor, &message_size, sizeof(int), 0, (struct sockaddr *) &server_address, sizeof(server_address));
            if (temp <= 0) {
                perror("ERROR writing in socket");
                continue;
            }
            temp = sendto(socket_descriptor, &full_buffer, message_size, 0, (struct sockaddr *) &server_address, sizeof(server_address));
            if (temp <= 0) {
                perror("ERROR writing in socket");
                continue;
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
    listen(socket_descriptor, 10);
    communicating();
    for (thread &t: threads_list) {
        t.join();
    }
}
