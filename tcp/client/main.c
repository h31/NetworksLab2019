#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include "../common-utils/headers/common.h"
#include "../common-utils/headers/inet_utils.h"
#include "../common-utils/headers/errors.h"
#include "../common-utils/headers/io.h"

#define INPUT_PROMT "message\n"
#define INPUT_PROMT_SIZE 9

void replace(char *buff, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (buff[i] == '\n') buff[i] = '\0';
    }
}

void sendm(int id, size_t size) {
    char message[size];
    bzero(message, size);
    fgets(message, (int) size, stdin);
    replace(message, size);
    send_message(id, message);
}

void *terminal(void *args) {
    int *client_id = (int *) args;
    for (;;) {
        char message[INPUT_PROMT_SIZE];
        fgets(message, INPUT_PROMT_SIZE, stdin);
        if (!strcmp(INPUT_PROMT, message)) {
            replace(message, 9);
            sendm(*client_id, MESSAGE_SIZE);
        }
    }
}

void start_client(int argc, char *argv[]) {
    socket_descriptor serv_addr;

    uint16_t portno = exclude_cliport(argc, argv);
    int sockfd = create_tcpsocket();
    host_description *server = gethostbyname(argv[1]);

    if (server == NULL) {
        raise_error(NO_SUCH_HOST);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (address *) &serv_addr, sizeof(serv_addr)) < 0) {
        raise_error(CONNECT_ERROR);
    }

    printf("Please enter your name: \n");

    /* firstly send client name */
    sendm(sockfd, CLIENT_NAME_SIZE);

    printf("Thank you. If you want to send a message type `message` \n");

    Client *this = empty_client(&sockfd);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, &terminal, &sockfd);

    for (;;) {
        Reader *reader = read_message(this);
        if (reader->exit_code == FAILURE) raise_error(SOCKET_READ_ERROR);
        else {
            printf("%s\n", reader->value);
            free_reader(reader);
        }
    }
}

int main(int argc, char *argv[]) {
    start_client(argc, argv);
}