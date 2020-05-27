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
#include <set>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <time.h>
#include <sstream>
#include <stdexcept>
#include <fstream>

using namespace std;

#define true  1
#define false 0
#define NUMBER_OF_CLIENTS 1000

/**********************************************************************************************************************
 *                                                  Global variables.                                                 *
 **********************************************************************************************************************/


int socket_descriptor;
map<string, string> clients_data;
map<string, int> clients;
map<string, string> clients_directory;
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

string to_str(int id) {
    stringstream ss;
    ss << id;
    return ss.str();
}

void authentication_data(){
    clients_data["q"] = "q";
    clients_data["test_user"] = "passwd";
    clients_data["Aleksej"] = "alexpass";
    clients_data["not_Aleksej"] = "alexpass";
}

list<string> split(const string &s, char delimiter) {
    list<string> tokens;
    string token;
    istringstream token_stream(s);
    while (getline(token_stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

string get_who(){
    string result = "";
    for (map<string,string>::iterator it = clients_directory.begin(); it != clients_directory.end(); it++){
        result = result + it->first;
        result = result + " ";
        result = result + it->second;
        result = result + "\n";
    }
    return result;
}

void kill_client(string name){
    for (map<string,string>::iterator it = clients_directory.begin(); it != clients_directory.end(); it++){
        if (it->first == name) {
            clients_directory.erase(it);
            break;
        }
    }
    for (map<string,int>::iterator it = clients.begin(); it != clients.end(); it++){
        if (it->first == name) {
            close(it->second);
            clients.erase(it);
            return;
        }
    }
}

string exec(string command) {
    char buffer[128];
    string result = "";

    // Open pipe to file
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "popen failed!";
    }

    // read till end of process:
    while (!feof(pipe)) {

        // use buffer to read and add to result
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }

    pclose(pipe);
    return result;
}

bool FileExists(const string& path){
    return ifstream(path.c_str()).good();
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

void communicating(int new_socket_descriptor);

void accept_connection() {
    struct sockaddr_in client_address;
    unsigned int client_length = sizeof(client_address);
    int nsd = accept(socket_descriptor, (struct sockaddr *) &client_address, &client_length);
    if (nsd <= 0) {
        perror("ERROR opening socket");
        return;
    }
    threads_list.emplace_back([&] { communicating(nsd); });
}

int read_int(int new_socket_descriptor){
    int message;
    int temp = read(new_socket_descriptor, &message, 4);
    if (temp <= 0) {
        perror("ERROR reading from socket");
        close(new_socket_descriptor);
        pthread_exit(0);
    }
    return message;
}

void write_int(int number){
    int temp = write(socket_descriptor, &number, 4);
    if (temp <= 0) {
        perror("ERROR reading from socket");
        shutdown(socket_descriptor, 2);
        close(socket_descriptor);
        pthread_exit(0);
    }
}

string read_message(int new_socket_descriptor){
    int message_size = read_int(new_socket_descriptor);
    auto buffer = new char[message_size];
    bzero(buffer, sizeof(buffer));
    int temp = read(new_socket_descriptor, buffer, message_size);
    if (temp <= 0) {
        perror("ERROR reading from socket");
        close(new_socket_descriptor);
        pthread_exit(0);
    }
    return string(buffer);
}

void write_message(int new_socket_descriptor, string message){
    int message_size = message.length();
    int temp = write(new_socket_descriptor, &message_size, 4);
    if (temp <= 0) {
        perror("ERROR writing in socket");
        pthread_exit(0);
    }
     temp = write(new_socket_descriptor, message.c_str(), message_size);
    if (temp <= 0) {
        perror("ERROR writing in socket");
        pthread_exit(0);
    }
}    

/**********************************************************************************************************************
 *                                                 IO File functions.                                                 *
 **********************************************************************************************************************/





/**********************************************************************************************************************
 *                                                IO-threads functions.                                               *
 **********************************************************************************************************************/


void communicating(int new_socket_descriptor) {
    string name = read_message(new_socket_descriptor);
    string pass = read_message(new_socket_descriptor);
    if (clients_data[name] != pass) pthread_exit(0);
    clients[name] = new_socket_descriptor;
    clients_directory[name] = "/home/lixir";
    while (true) {
        string request = read_message(new_socket_descriptor);
        list<string> request_split = split(request, ' ');
        string response = "";
        if (request_split.front() == "ls") {
            response = exec("ls " + clients_directory[name]);
        } else if (request_split.front() == "cd") {
            if (FileExists(request_split.back())) {
                response = "Success";
                clients_directory[name] = request_split.back();
            } else {
                response = "Failed";
            }
        } else if (request_split.front() == "who") {
            response = get_who();
        } else if (request_split.front() == "kill") {
            kill_client(request_split.back());
            response = request_split.back();
            response = response + " killed.";
        } else if (request_split.front() == "logout") {
            kill_client(name);
            pthread_exit(0);
        }
        write_message(new_socket_descriptor, response);
    }
    pthread_exit(0);
}

/**********************************************************************************************************************
 *                                                    main-function.                                                  *
 **********************************************************************************************************************/


int main(int argc, char *argv[]) {
    check_argc(argc, argv);
    authentication_data();
    initialization_socket_descriptor();
    initialization_socket_structure();
    bind_host_address();
    listen(socket_descriptor, 5);
    while (true) {
        accept_connection();
    }
    for (thread &t: threads_list) {
        t.join();
    }
}
