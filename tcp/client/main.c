#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "../common.h"

#define MAX_MESSAGE_SIZE 256
#define HEADER_SIZE 4

char *user_name;
int global_sockfd;

/*------------------- CLIENT SEND MESSAGE ---------------------------------*/
void client_send_message(int sockfd, Message message) {
    if (write(sockfd, &message.size, sizeof(int)) <= 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }

    if (write(sockfd, message.buffer, message.size) <= 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }

}

void client_send_message_from_cmd(int sockfd) {
    Message message;
    message.buffer = malloc(MAX_MESSAGE_SIZE * sizeof(char));
    bzero(message.buffer, MAX_MESSAGE_SIZE);
    printf("\n%s:", user_name);
    fgets(message.buffer, MAX_MESSAGE_SIZE, stdin);
    message.size = (int) strlen(message.buffer);

    client_send_message(sockfd, message);

}

void client_send_message_loop(int sockfd) {
    for (;;) {
        client_send_message_from_cmd(sockfd);
    }
}

/*------------------- CLIENT GET RESPONSE ---------------------------------*/
void client_get_response(int sockfd) {
    Message message;
    if (read(sockfd, &message.size, HEADER_SIZE) <= 0) {
        PERROR_AND_EXIT("ERROR reading message size");
    }

    message.buffer = calloc(message.size, sizeof(char));

    if (read(sockfd, message.buffer, message.size) <= 0) {
        PERROR_AND_EXIT("ERROR reading message")
    }

    printf("\r%s\n%s:", message.buffer, user_name);
    fflush(stdout);
}

void client_get_response_loop(int sockfd) {
    for (;;) {
        client_get_response(sockfd);
    }
}

/*------------------- CLIENT SIGINT HANDLER --------------------------------------------*/
void client_sigint_handler(int signo) {
    printf("Closing client...\n");
    close(global_sockfd);
    free(user_name);
    printf("Client close...\n");
    exit(0);
}

/*------------------- MAIN -----------------------------------------------------*/
int main(int argc, char *argv[]) {
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server = NULL;
    int sockfd;

    int opt;
    while ((opt = getopt(argc, argv, "i:p:n:")) != -1) {
        switch (opt) {
            case 'i':
                server = gethostbyname(optarg);
                if (server == NULL) {
                    fprintf(stderr, "ERROR, no such host\n");
                    exit(0);
                }
                break;
            case 'p':
                portno = (uint16_t) atoi(optarg);
                break;
            case 'n':
                user_name = strdup(optarg);
                break;
            default:
                /* unrecognised opt ... add your error condition */
                printf("Unrecognized opt = %s", optarg);
                exit(1);
        }
    }
    if (signal(SIGINT, client_sigint_handler) == SIG_ERR) {
        PERROR_AND_EXIT("client_sigint_handler initialized failed");
    }

    /* Create a socket point */
    global_sockfd = sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        PERROR_AND_EXIT("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        PERROR_AND_EXIT("ERROR connecting");
    }
    printf("Conneced to server \n");

    client_send_message(sockfd, *get_new_message(user_name));

    pthread_t read_thread;
    pthread_create(&read_thread, NULL, (void *) client_get_response_loop, (void *) sockfd);

    client_send_message_loop(sockfd);

    return 0;
}