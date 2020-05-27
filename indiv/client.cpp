/**==================================================================================================================**
 | @name Sergeev A.A.                                                                                                 |
 | @group #3530901/60201                                                                                              |
 | @task #1-TCP                                                                                                       |
 | @file client.cpp                                                                                                   |
 **==================================================================================================================**/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <iostream>
#include <locale.h>
#include <string>

#define true  1
#define false 0
#define BUFFER_LENGTH 1024

using namespace std;

/**********************************************************************************************************************
 *                                                  Global variables.                                                 *
 **********************************************************************************************************************/

int socket_descriptor;
uint16_t port_number;
struct sockaddr_in server_address;
struct hostent *server;
char temp_string[4];
string output_buffer;

pthread_mutex_t cs_mutex;

time_t seconds;

/**********************************************************************************************************************
 *                                                Auxiliary functions.                                                *
 **********************************************************************************************************************/

void check_condition(int cond, char *str, int sig){
    if (cond){
        perror(str);
        exit(sig);
    }
}

void check_argc(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage %s hostname port\n", argv[0]);
        exit(0);
    }
    server = gethostbyname(argv[1]);
    check_condition(server == NULL, "ERROR, no such host\n", 0);

    // convert to network-short (to big-endian)
    port_number = htons((uint16_t) atoi(argv[2]));
}

/**********************************************************************************************************************
 *                                               TCP-connect functions.                                               *
 **********************************************************************************************************************/

void initialization_socket_descriptor(){
    //  AF_INET     - IPv4
    //  SOCK_STREAM - TCP
    //  0           - Default
    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
}

void initialization_server_address(){
    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &server_address.sin_addr.s_addr, (size_t) server->h_length);
    server_address.sin_port = port_number;
}

void server_connect(){
    if (connect(socket_descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
}

int read_int(){
    int message;
    int temp = read(socket_descriptor, &message, 4);
    if (temp <= 0) {
        perror("ERROR reading from socket");
        shutdown(socket_descriptor, 2);
        close(socket_descriptor);
        exit(0);
    }
    return message;
}

void write_int(int number){
    int temp = write(socket_descriptor, &number, 4);
    if (temp <= 0) {
        perror("ERROR reading from socket");
        shutdown(socket_descriptor, 2);
        close(socket_descriptor);
        exit(0);
    }
}

string read_message(){
    int message_size;
    int temp = read(socket_descriptor, &message_size, 4);
    if (temp <= 0) {
        perror("ERROR reading from socket");
        shutdown(socket_descriptor, 2);
        close(socket_descriptor);
        exit(0);
    }
    auto buffer = new char[message_size];
    bzero(buffer, sizeof(buffer));
    temp = read(socket_descriptor, buffer, message_size);
    if (temp <= 0) {
        perror("ERROR reading from socket");
        shutdown(socket_descriptor, 2);
        close(socket_descriptor);
        pthread_exit(0);
    }
    return string(buffer);
}

void write_message(string message){
    int message_size = message.length();
    int temp = write(socket_descriptor, &message_size, 4);
    if (temp <= 0) {
        perror("ERROR writing in socket");
        shutdown(socket_descriptor, 2);
        close(socket_descriptor);
        exit(0);
    }
    temp = write(socket_descriptor, message.c_str(), message_size);
    if (temp <= 0) {
        perror("ERROR writing in socket");
        shutdown(socket_descriptor, 2);
        close(socket_descriptor);
        exit(0);
    }
}

/**********************************************************************************************************************
 *                                                IO-threads functions.                                               *
 **********************************************************************************************************************/


void * output_thread_fun(){
    printf("Please, enter login:\n");
    string name = "";
    getline(cin,name,'\n');
    printf("Please, enter password:\n");
    string pass = "";
    getline(cin,pass,'\n');
    cin.clear();
    write_message(name);
    write_message(pass);

    while(true){
        printf("[%s@server] ", name.c_str());
        string request = "";
        getline(cin,request,'\n');
        write_message(request);
        string response = read_message();
        printf("%s\n", response.c_str());
    }
}

/**********************************************************************************************************************
 *                                                    main-function.                                                  *
 **********************************************************************************************************************/

int main(int argc, char *argv[]) {
    check_argc(argc, argv);
    initialization_socket_descriptor();
    initialization_server_address();
    server_connect();

    thread output_thread(output_thread_fun);

    output_thread.join();
}

