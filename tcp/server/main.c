#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include "protocol.h"


#define CONNECTION_PORT 5001
#define MAX_CLIENTS 32

#define GREETING_MSG_LEN 40

#define STATUS_NEW 0
#define STATUS_OK 1
#define STATUS_DISCONNECTED -1

#define EXIT_OK 0
#define EXIT_ER 1
#define INVALID -1


int add_client(int sock);
int read_name(int sock, char* name_buf);
int assign_client_name(int cl_id, int cl_sock, char* cl_name);
void remove_client(int id);
void greeting(int cl_id);
int broadcast(int id, char* name, char* buffer);
void* th_client_handler(void* args);
void sig_close_handler(int sig);

struct Client
{
    char* name;
    int sock;
    int status;
};

struct ServerData
{
    struct Client clients[MAX_CLIENTS];
    int counter;
} clients_data;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int server_sockfd;


int main(int argc, char *argv[])
{
    int client_sockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;

    /* First call to socket() function */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(EXIT_ER);
    }

    /* Initialize socket structure */
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = CONNECTION_PORT;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(EXIT_ER);
    }
    signal(SIGINT, sig_close_handler);
    printf("Server is ready!\n");

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */
    listen(server_sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1)
    {
        /* Accept actual connection from the client */
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (client_sockfd < 0)
        {
            perror("ERROR on accept");
            exit(EXIT_ER);
        }

        int new_cl_id = add_client(client_sockfd);
        if (new_cl_id == INVALID) continue;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, th_client_handler, &new_cl_id) < 0)
        {
            perror("ERROR on thread create");
            exit(EXIT_ER);
        }
    }
    return EXIT_OK;
}

int add_client(int sock)
{
    pthread_mutex_lock(&mutex);
    int id = clients_data.counter;
    for (int i = 0; i < clients_data.counter; ++i)
    {
        if (clients_data.clients[i].status == STATUS_DISCONNECTED)
        {
            id = i;
            break;
        }        
    }
    if (id >= MAX_CLIENTS)
    {
        perror("Too many clients");
        shutdown(sock, SHUT_RDWR);
        close(sock);
        return INVALID;
    }    
    clients_data.clients[id].sock = sock;
    clients_data.clients[id].status = STATUS_NEW;
    clients_data.counter++;
    pthread_mutex_unlock(&mutex);
    return id;
}

int read_name(int sock, char* name_buf)
{
    read_msg(sock, name_buf);
    printf("New user: %s\n", name_buf);
    return EXIT_OK;
}

int assign_client_name(int cl_id, int cl_sock, char* cl_name)
{
    int check = read_name(cl_sock, cl_name);
    pthread_mutex_lock(&mutex);
    clients_data.clients[cl_id].name = cl_name;
    pthread_mutex_unlock(&mutex);
    if (check < 0)
    {
        printf("ERROR on client add\n");
    }
    return check;
}

int broadcast(int id, char* name, char* buffer){
    char* full_msg[FULL_MSG_LEN] = {0};
    char* full_back_msg[FULL_MSG_LEN] = {0};
    char* name_self = "YOU";
    build_msg(full_msg, name, buffer);
    build_msg(full_back_msg, strdup(name_self), buffer);

    if (clients_data.clients[id].status == STATUS_NEW)
    {        
        greeting(id);             
    }
    for (int i = 0; i < clients_data.counter; ++i)
    {   
        if (clients_data.clients[i].status == STATUS_DISCONNECTED) continue;
        if (i != id)
        {
            send_msg(clients_data.clients[i].sock, full_msg);       
        }
        else
        {
            send_msg(clients_data.clients[i].sock, full_back_msg);
        }            
    }
}

void greeting(int cl_id)
{
    char* full_msg[FULL_MSG_LEN] = {0};
    char* greeting_msg[GREETING_MSG_LEN] = {0};
    char* name_server = "SERVER";
    sprintf(greeting_msg, "Welcome to the chat, %s!", clients_data.clients[cl_id].name);   

    pthread_mutex_lock(&mutex);
    build_msg(full_msg, strdup(name_server), greeting_msg);    
    send_msg(clients_data.clients[cl_id].sock, full_msg);
    clients_data.clients[cl_id].status = STATUS_OK;
    pthread_mutex_unlock(&mutex);
}

void remove_client(int id)
{
    pthread_mutex_lock(&mutex);
    int client_sockfd = clients_data.clients[id].sock;
    shutdown(client_sockfd, SHUT_RDWR);  
    close(client_sockfd);
    clients_data.clients[id].status = STATUS_DISCONNECTED;
    clients_data.counter--;
    pthread_mutex_unlock(&mutex);
}

void* th_client_handler(void *args)
{
    // client data handler
    pthread_mutex_lock(&mutex); 
    int id = *(int *) args;
    int sock = clients_data.clients[id].sock;
    pthread_mutex_unlock(&mutex);
    char name[MAX_NAME_LEN];
    assign_client_name(id, sock, name);

    // client's msg handler
    char msg_buf[MAX_MSG_LEN];
    char* full_msg[FULL_MSG_LEN] = {0};
    while (read_msg(sock, msg_buf) > 0)
    {
        build_msg(full_msg, name, msg_buf);
        printf("%s\n", full_msg);
        broadcast(id, name, msg_buf);        
        fflush(stdout);
        bzero(msg_buf, sizeof(msg_buf));
        bzero(full_msg, sizeof(full_msg));
    }
    remove_client(id); 
}

void sig_close_handler(int sig)
{
    if (sig != SIGINT) return;
    printf("\nClosing clients connections..");
    for (int i = 0; i < clients_data.counter; ++i)
    {   
        remove_client(i);
    }
    printf("\nClosing server..\n");
    shutdown(server_sockfd, SHUT_RDWR);
    close(server_sockfd);
    pthread_mutex_destroy(&mutex);
}
