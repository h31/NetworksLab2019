#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <signal.h>
#include "../common.h"
#include "pthread.h"

#define MAX_CLIENT_NUM      4
#define HEADER_SIZE         4
#define MAX_MESSAGE_SIZE    256
#define CL_STATUS_DISC      0
#define CL_STATUS_CONN      1
#define NO_PLACES_INDEX     0
#define NO_PLACES_STRING    "Sorry, no places\n"


Client clients[MAX_CLIENT_NUM];
int global_sockfd;

/*------------------- GETTERS --------------------------------------------*/
int get_free_index() {
    for (int i = 1; i < MAX_CLIENT_NUM; ++i) {
        if (clients[i].status == CL_STATUS_DISC)
            return i;
    }
    return NO_PLACES_INDEX;
}

int get_i_from_sockfd(int sockfd) {
    for (int i = 1; i < MAX_CLIENT_NUM; ++i) {
        if (clients[i].sockfd == sockfd)
            return i;
    }
    return -1;
}

/*------------------- SERVER DELETE CLIENT ---------------------------------*/
void server_delete_client(int i) {
    printf("Delete client: name = %s, i = %d, sockfd = %d\n", clients[i].name, i, clients[i].sockfd);
    close(clients[i].sockfd);
    free(clients[i].name);
    clients[i].status = CL_STATUS_DISC;
    pthread_cancel(clients[i].thread);

}

/*------------------- SERVER GET MESSAGE ---------------------------------*/
Message serv_get_message(int sockfd) {
    Message message;

    message.buffer = malloc(MAX_MESSAGE_SIZE * sizeof(char));
    bzero(message.buffer, MAX_MESSAGE_SIZE);
    message.size = 0;

    if (read(sockfd, &message.size, sizeof(int)) <= 0) {
        server_delete_client(get_i_from_sockfd(sockfd));
    }
    if (read(sockfd, message.buffer, message.size) <= 0) {
        server_delete_client(get_i_from_sockfd(sockfd));
    }
    return message;
}

/*------------------- SERVER SEND MESSAGE ---------------------------------*/
void serv_send_response(int sockfd, Message message) {

    if (write(sockfd, &message.size, HEADER_SIZE) <= 0) {
        server_delete_client(get_i_from_sockfd(sockfd));
    }

    if (write(sockfd, message.buffer, message.size) <= 0) {
        server_delete_client(get_i_from_sockfd(sockfd));
    }
}

/*------------------- SERVER PROCESS CLIENT -------------------------------------*/
void serv_process_client(int curr_i) {
    for (;;) {
        Message message = serv_get_message(clients[curr_i].sockfd);
        printf("RECEIVED:%s:message = %s(size = %d)\n", clients[curr_i].name, message.buffer, message.size);
        message.buffer = str_concat(str_concat(clients[curr_i].name, ":"), message.buffer);
        message = *NewMessage(message.buffer);

        for (int j = 0; j < MAX_CLIENT_NUM; ++j) {
            if (clients[j].status == CL_STATUS_CONN && j != curr_i) {
                serv_send_response(clients[j].sockfd, message);
                printf("SENDED:message = %s(size = %d) to %s\n",
                       message.buffer,
                       message.size,
                       clients[j].name);

            }
        }

    }

}

/*------------------- SERVER SIGINT HANDLER --------------------------------------------*/
void server_sigint_handler(int signo) {
    printf("Closing server...\n");
    for (int i = 0; i < MAX_CLIENT_NUM; ++i) {
        if (clients[i].status == CL_STATUS_CONN) {
            server_delete_client(i);
        }
    }
    printf("Close socket = %d\n", global_sockfd);
    close(global_sockfd);
    printf("Server closed...\n");
    exit(0);
}


/*------------------- MAIN -----------------------------------------------------*/
int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;

    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                portno = (uint16_t) atoi(optarg);
                break;
            default:
                /* unrecognised opt ... add your error condition */
                printf("Unrecognized opt = %s", optarg);
                break;
        }
    }
    if (signal(SIGINT, server_sigint_handler) == SIG_ERR) {
        PERROR_AND_EXIT("client_sigint_handler initialized failed");
    }

    /* First call to socket() function */
    global_sockfd = sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        PERROR_AND_EXIT("ERROR opening socket");
    }

    printf("Portno = %d\n", portno);
    printf("Sockfd = %d\n", sockfd);
    printf("IP ADDRESS:\n");
    system(" ifconfig | sed -En 's/127.0.0.1//;s/.*inet (addr:)?(([0-9]*\\.){3}[0-9]*).*/\\2/p'");




    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        PERROR_AND_EXIT("ERROR on binding");
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    for (;;) {
        /* Accept actual connection from the client */

        int i = get_free_index();
        clients[i].sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (clients[i].sockfd <= 0) {
            PERROR_AND_EXIT("ERROR on accept");
        }
        if (i == NO_PLACES_INDEX) {
            printf("Sending no cap message to i = %d, sockfd = %d\n", i, clients[i].sockfd);
            serv_send_response(clients[i].sockfd, *NewMessage(NO_PLACES_STRING));
            server_delete_client(i);
        } else {
            Message name_mess = serv_get_message(clients[i].sockfd);
            clients[i].name = name_mess.buffer;

            clients[i].status = CL_STATUS_CONN;
            printf("New client accepted: name = %s, i = %d, sockfd = %d\n", clients[i].name, i, clients[i].sockfd);
            pthread_create(&clients[i].thread, NULL, (void *) serv_process_client, (void *) i);
        }
    }

    return 0;
}
