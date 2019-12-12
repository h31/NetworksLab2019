#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <termios.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include "protocol.h"


#define CONNECTION_PORT 5001
#define MAX_CLIENTS 64

#define GREETING_MSG_LEN 40

#define STATUS_NEW 0
#define STATUS_OK 1
#define STATUS_DISCONNECTED -1

#define MODE_LISTEN 0
#define MODE_WRITE 1

#define EXIT_OK 0
#define EXIT_ER 1
#define INVALID -1


void init_mode();
void set_mode(int new_mode);
void fds_init();
void add_fds(int sockfd);
int add_client(int sock);
int read_name(int sock, char* name_buf);
int assign_client_name(int cl_id, int cl_sock, char* cl_name);
void remove_client(int fd_id);
void greeting(int fd_id);
int broadcast(int fd_id, char* name, char* buffer);
void client_handler(int fd_id);
void sig_close_handler(int sig);
void close_server();

struct Client
{
    char* name;
    int status;
};

struct ServerData
{
    struct Client clients[MAX_CLIENTS];
    int counter;
} clients_data;

int server_sockfd;
struct pollfd *fds;
int mode;
static struct termios settings_write;
static struct termios settings_listen;

int main(int argc, char *argv[])
{
    int client_sockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;
    int on = 1;
    char name[MAX_NAME_LEN];

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

    /* Allow socket descriptor to be reuseable */
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0)
    {
        perror("ERROR on setsockopt");
        exit(EXIT_ER);
    }

    /* Set socket to be nonblocking. All of the sockets for      
       the incoming connections will also be nonblocking */
    if (ioctl(server_sockfd, FIONBIO, (char *)&on) < 0)
    {
        perror("ioctl() failed");
        close(server_sockfd);
        exit(EXIT_ER);
    }

    /* Now bind the host address using bind() call.*/
    if (bind(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(EXIT_ER);
    }
    signal(SIGINT, sig_close_handler);

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */
    listen(server_sockfd, 5);
    clilen = sizeof(cli_addr);

    fds_init();
    init_mode(); 
    set_mode(MODE_LISTEN);
    printf("Server is ready!\n");

    int status;
    while (1)
    {
        status = poll(fds, clients_data.counter, -1);
        if (status < 0)
        {
            perror("ERROR on poll");
            close_server();
        }

        for (int i = 0; i < clients_data.counter; i++)
        {
            if (fds[i].revents == 0) continue;
            if (fds[i].revents != POLLIN)
            {
                perror("ERROR on revents");
                close_server();
            }
            if (fds[i].fd == server_sockfd)
            {
                while (1)
                {
                    client_sockfd = accept(server_sockfd, (struct sockaddr *)&cli_addr, &clilen);

                    if (client_sockfd < 0) break;
                    
                    // new client                
                    int new_cl_id = add_client(client_sockfd);
                    if (new_cl_id != INVALID)
                    {
                        add_fds(client_sockfd);

                        bzero(name, sizeof(name));
                        assign_client_name(new_cl_id, client_sockfd, name);
                    }
                }
            }
            else
            {
                client_handler(i);
            }            
        }        
    }
    close_server();
    return EXIT_OK;
}

void init_mode()
{ 
    tcgetattr(0, &settings_write);
 
    settings_listen = settings_write;
 
    settings_listen.c_lflag &= (~ICANON);
    settings_listen.c_lflag &= (~ECHO);
    settings_listen.c_cc[VTIME] = 0;
    settings_listen.c_cc[VMIN] = 1;
}

void set_mode(int new_mode)
{
    if (new_mode == MODE_WRITE)
    {
        tcsetattr(0, TCSANOW, &settings_write);
    }
    else if (new_mode == MODE_LISTEN)
    {
        tcsetattr(0, TCSANOW, &settings_listen);
    }
    else
    {
        printf("\nInvalid client mode.\n");
        return;
    }    
    mode = new_mode;
}

void fds_init()
{
    /* phantom client */
    add_client(server_sockfd);
    clients_data.clients[0].name = "SERVER";

    /* poll init */
    fds = (struct pollfd *) malloc(sizeof(struct pollfd));
    bzero(fds, sizeof(fds));
    fds[0].fd = server_sockfd;
    fds[0].events = POLLIN;
}

void add_fds(int sockfd)
{
    fds = (struct pollfd *) realloc(fds, clients_data.counter * sizeof(struct pollfd));

    fds[clients_data.counter - 1].fd = sockfd;
    fds[clients_data.counter - 1].events = POLLIN;
}

int add_client(int sock)
{
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
    clients_data.clients[id].status = STATUS_NEW;
    clients_data.counter++;
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
    bzero(cl_name, sizeof(cl_name));
    int check = read_name(cl_sock, cl_name);
    clients_data.clients[cl_id].name = cl_name;
    if (check < 0)
    {
        printf("ERROR on client add\n");
    }
    return check;
}

int broadcast(int fd_id, char* name, char* buffer)
{
    char* full_msg[FULL_MSG_LEN] = {0};
    char* full_back_msg[FULL_MSG_LEN] = {0};
    char* name_self = "YOU";
    build_msg(full_msg, name, buffer);
    build_msg(full_back_msg, strdup(name_self), buffer);

    if (clients_data.clients[fd_id].status == STATUS_NEW)
    {        
        greeting(fd_id);             
    }
    for (int i = 1; i < clients_data.counter; ++i)
    {   
        int cl_sock = fds[i].fd;
        if (clients_data.clients[i].status == STATUS_DISCONNECTED) continue;
        if (i != fd_id)
        {
            send_msg(cl_sock, full_msg);       
        }
        else
        {
            send_msg(cl_sock, full_back_msg);
        }            
    }
    bzero(full_msg, sizeof(full_msg));
    bzero(full_back_msg, sizeof(full_back_msg));
}

void greeting(int fd_id)
{
    char* full_msg[FULL_MSG_LEN] = {0};
    char* greeting_msg[GREETING_MSG_LEN] = {0};
    char* name_server = "SERVER";
    sprintf(greeting_msg, "Welcome to the chat, %s!", clients_data.clients[fd_id].name);   

    build_msg(full_msg, strdup(name_server), greeting_msg);    
    send_msg(fds[fd_id].fd, full_msg);
    clients_data.clients[fd_id].status = STATUS_OK;
}

void remove_client(int fd_id)
{
    // clients update
    int client_sockfd = fds[fd_id].fd;
    shutdown(client_sockfd, SHUT_RDWR);  
    close(client_sockfd);
    clients_data.clients[fd_id].status = STATUS_DISCONNECTED;
    
    // fd update
    int fds_size = clients_data.counter;
    for (int i = 0; i < fds_size; i++)
    {
        if (fds[i].fd == client_sockfd) 
        {
            for (int j = i; j < fds_size; j++) 
            {
                if (j != fds_size - 1) 
                {
                    fds[j] = fds[j + 1];
                }
            }
            clients_data.counter--;
            fds = (struct pollfd *) realloc(fds, fds_size * sizeof(struct pollfd));
            break;
        }
    }
}

void client_handler(int fd_id)
{
    int status;
    int sock = fds[fd_id].fd;
    char* name = clients_data.clients[fd_id].name;
    
    status = poll(&fds[fd_id], 1, -1);
    
    if (status < 0)
    {
        perror("ERROR on poll");
        exit(EXIT_ER);
    }
    if (fds[fd_id].revents != POLLIN)
    {
        perror("ERROR on revents");
        exit(EXIT_ER);
    }

    // client's msg handler
    char msg_buf[MAX_MSG_LEN];
    char* full_msg[FULL_MSG_LEN] = {0};
    bzero(msg_buf, sizeof(msg_buf));
    bzero(full_msg, sizeof(full_msg));
    
    status = read_msg(sock, msg_buf);
    if (status < 0) 
    {
        remove_client(fd_id);
    }
    if (strcmp(msg_buf, "\0") != 0)
    {
        build_msg(full_msg, name, msg_buf);
        printf("%s\n", full_msg);
        broadcast(fd_id, name, msg_buf);        
    }
    
    fflush(stdout);
    bzero(msg_buf, sizeof(msg_buf));
    bzero(full_msg, sizeof(full_msg));
}

void sig_close_handler(int sig)
{
    if (sig != SIGINT) return;
    close_server();
}

void close_server()
{
    printf("\nClosing clients connections..");
    for (int i = 1; i < clients_data.counter; ++i)
    {   
        remove_client(i);
    }
    printf("\nClosing server..\n");
    shutdown(server_sockfd, SHUT_RDWR);
    close(server_sockfd);
    set_mode(MODE_WRITE);
    exit(EXIT_OK);
}