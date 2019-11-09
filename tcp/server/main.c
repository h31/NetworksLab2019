#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <signal.h>
#include "../common.h"
#include <fcntl.h>

#define HEADER_SIZE         4
#define MAX_MESSAGE_SIZE    256
#define POLL_TIMEOUT        -1
#define POLL_INIT_SIZE      10
#define POLL_INCREASE_LEN   10

Client *first_client;
int global_sockfd;
PollfdList *pollfd_list;

void server_sigint_handler(int signo);


/*------------------- SERVER WORK WITH POLLFDS LIST ---------------------------------*/
void server_init_pollfd_list() {
    free(pollfd_list);
    pollfd_list = calloc(1, sizeof(PollfdList));
    pollfd_list->max_size = POLL_INIT_SIZE;
    pollfd_list->pollfds = calloc(pollfd_list->max_size, sizeof(struct pollfd));
    pollfd_list->pollfds[0].fd = global_sockfd;
    pollfd_list->pollfds[0].events = POLLIN;
    pollfd_list->size = 1;

}

void server_update_pollfd_list() {
    server_init_pollfd_list();

    printf("--------------------------------\n");
    printf("Clients in list:\n");
    Client *another_client = first_client->next_client;
    int i = 1;
    while (another_client != NULL) {
        if (i == pollfd_list->max_size) {
            pollfd_list->max_size += POLL_INCREASE_LEN;
            pollfd_list->pollfds = realloc(pollfd_list->pollfds, pollfd_list->max_size);
            PRETTY_MARK_AND_DO(
                    printf("NEW POLL MAX_SIZE = %d\n", pollfd_list->max_size);
            );
        }
        __uint32_t cli_addr = another_client->sockaddr->sin_addr.s_addr;
        printf("%d:(%d.%d.%d.%d)\t:%s\t:%d\n", i,
               (u_char) cli_addr, (u_char) (cli_addr >> 8), (u_char) (cli_addr >> 16),
               (u_char) (cli_addr >> 24),
               another_client->name,
               another_client->sockfd);

        pollfd_list->pollfds[i].fd = another_client->sockfd;
        pollfd_list->pollfds[i].events = POLL_IN;
        pollfd_list->size++;
        another_client = another_client->next_client;
        i++;
    }
    printf("--------------------------------\n");


}

/*------------------- SERVER DELETE CLIENT ---------------------------------*/
void server_delete_client(Client *client) {
    printf("Delete client: name = %s, sockfd = %d\n", client->name, client->sockfd);
    close(client->sockfd);
    if (client->prev_client != NULL) {
        printf("prev = %s, ", client->prev_client->name);
        client->prev_client->next_client = client->next_client;
    }
    if (client->next_client != NULL) {
        printf("next = %s", client->next_client->name);
        client->next_client->prev_client = client->prev_client;
    }
    printf("\n");
    free(client);
    server_update_pollfd_list();

}

/*------------------- SERVER GET MESSAGE ---------------------------------*/
Message *serv_get_message(Client *client) {
    Message *message = calloc(1, sizeof(Message));

    message->err = message->size = 0;

    if (read(client->sockfd, &message->size, HEADER_SIZE) <= 0) {
        message->err = 1;
        return message;
    }

    message->buffer = calloc(message->size, sizeof(char));

    if (readN(client->sockfd, message->buffer, message->size) <= 0) {
        message->err = 1;
        return message;
    }
    return message;
}

/*------------------- SERVER SEND MESSAGE ---------------------------------*/
void serv_send_response(Client *client, Message *message) {

    if (write(client->sockfd, &message->size, HEADER_SIZE) <= 0) {
        server_delete_client(client);
        return;
    }

    if (write(client->sockfd, message->buffer, message->size) <= 0) {
        server_delete_client(client);
        return;
    }
}

/*------------------- SERVER PROCESS CLIENT -------------------------------------*/
void serv_process_client(Client *client) {

    Message *message = serv_get_message(client);
    if (message->err == 0) {
        printf("\nRECEIVED:%s:message = %s(size = %d)\n", client->name, message->buffer, message->size);
        message->buffer = str_concat(str_concat(client->name, ":"), message->buffer);
        message = get_new_message(message->buffer);

        Client *another_client = first_client->next_client;
        while (another_client != NULL) {

            if (another_client != client &&
                another_client->status == CL_STAT_ON) {

                serv_send_response(another_client, message);
                printf("SENDED:message = %s(size = %d) to %s\n",
                       message->buffer,
                       message->size,
                       another_client->name);

            }

            another_client = another_client->next_client;
        }

    } else {
        printf("Bad message from client: name = %s, sockfd = %d\n", \
                client->name, client->sockfd);
        server_delete_client(client);
    }
    free(message->buffer);
    free(message);

}

/*------------------- SERVER SIGINT HANDLER --------------------------------------------*/
void server_sigint_handler(int signo) {
    printf("Closing server...\n");
    Client *deleted_client = first_client;
    if (deleted_client != NULL) {
        while (deleted_client->next_client != NULL) {
            deleted_client = deleted_client->next_client;
            server_delete_client(deleted_client->prev_client);
        }
    }
    server_delete_client(deleted_client);

    printf("Close socket = %d\n", global_sockfd);
    close(global_sockfd);
    printf("Server closed...\n");
    exit(0);
}

/*------------------- GET LAST NOT NULL CLIENT ----------------------------------------------*/
Client *get_last_not_null_client() {
    Client *free_client = first_client;
    int i = 1;
    while (free_client->next_client != NULL) {
        free_client = free_client->next_client;
        i++;
    }
    return free_client;

}

/*------------------- MAIN -----------------------------------------------------*/
int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int newsockfd = 0;

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

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        PERROR_AND_EXIT("ERROR making socket nonblock");
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) {
        PERROR_AND_EXIT("setsockopt(SO_REUSEADDR) failed");
    }

    first_client = get_new_client_empty();
    first_client->name = "first client";

    server_init_pollfd_list();


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

    /* Now start listening for the first_client, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    for (;;) {
        PRETTY_MARK_AND_DO(
                printf("Checking main poll...\n");
                if (poll(pollfd_list->pollfds, pollfd_list->size, POLL_TIMEOUT) < 0) {
                    perror("ERROR on first poll");
                    break;
                }
                printf("Something happened...\n");
        )

        int temp_size = pollfd_list->size;
        Client *processed_client = first_client->next_client;
        for (int i = 0; i < temp_size; i++) {
            if (i == 0) {//accepting new clients mode
                for (;;) {

                    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

                    if (newsockfd <= 0) {
                        break;
                    }

                    Client *free_client = get_last_not_null_client();
                    Client *new_client = get_new_client_empty();
                    free_client->next_client = new_client;
                    new_client->prev_client = free_client;

                    new_client->sockfd = newsockfd;

                    Message *name_mess = serv_get_message(new_client);
                    new_client->name = name_mess->buffer;
                    new_client->sockaddr = &cli_addr;
                    new_client->status = CL_STAT_ON;
                    printf("New client accepted: name = %s, sockfd = %d\n", new_client->name,
                           new_client->sockfd);

                    server_update_pollfd_list();
                }
                printf("All new clients accepted\n");
            } else {//processing clients mode

                if (pollfd_list->pollfds[i].revents == 0) {//nothing to do
                    processed_client = processed_client->next_client;
                    continue;

                } else if (pollfd_list->pollfds[i].revents == POLLIN) {//read message from clients
                    /*awaiting for message.buffer after message.size sending*/
                    printf("Checking poll in message processing...\n");
                    if (poll(&pollfd_list->pollfds[i], 1, POLL_TIMEOUT) < 0) {
                        perror("ERROR on second poll");
                        break;
                    }
                    printf("Processed client = %s, revent = %d\n", processed_client->name,
                           pollfd_list->pollfds[i].revents);
                    serv_process_client(processed_client);

                } else if (pollfd_list->pollfds[i].revents == POLL_ERR) {//bad cases
                    printf("Bad client = %s, revent = POLL_ERR\n", processed_client->name);
                    server_delete_client(processed_client);

                } else if (pollfd_list->pollfds[i].revents == POLL_HUP) {//bad cases
                    printf("Bad client = %s, revent = POLL_HUP\n", processed_client->name);
                    server_delete_client(processed_client);

                } else {//bad cases
                    printf("Unterminated case\n");
                    server_sigint_handler(SIGINT);
                }
                processed_client = processed_client->next_client;
            }
        }
    }

    return 0;
}
