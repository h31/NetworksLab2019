#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#define PERROR_AND_EXIT(message){\
    perror("ERROR opening socket");\
    exit(1);}
#define MAX_CLIENT_NUM 4
#define HEADER_SIZE 4
#define MAX_MESSAGE_SIZE 256

struct Client {
    int sockfd;
    char *name;

} typedef Client;

Client clients[MAX_CLIENT_NUM];

void serv_get_message(int sockfd, char *buffer) {
    char buffer_header[HEADER_SIZE];

    if (read(sockfd, buffer_header, HEADER_SIZE) <= 0) {
        PERROR_AND_EXIT("ERROR reading message size");
    }

    int message_size = atoi(buffer_header);
    printf("message_size = %d", message_size);

    buffer = calloc(message_size, sizeof(char));

    if (read(sockfd, buffer, message_size) <= 0) {
        PERROR_AND_EXIT("ERROR reading message")
    }

    printf("message = %s", buffer);

}

void serv_send_response(int sockfd, char *buffer) {
    
    if (write(sockfd, (int) strlen(buffer), sizeof(int)) <= 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }

    if (write(sockfd, buffer, strlen(buffer)) <= 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }


}

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

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        PERROR_AND_EXIT("ERROR opening socket");
    }

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


    Client client;
    /* Accept actual connection from the client */
    client.sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (client.sockfd < 0) {
        PERROR_AND_EXIT("ERROR on accept");
    }


    while (1) {
        char *buffer;
        serv_get_message(client.sockfd, buffer);

        serv_send_response(client.sockfd, buffer);
    }

    return 0;
}
