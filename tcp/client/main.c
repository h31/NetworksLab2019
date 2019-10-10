#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#define PERROR_AND_EXIT(message){\
    perror("ERROR opening socket");\
    exit(1);}

#define MAX_MESSAGE_SIZE 256
#define HEADER_SIZE 4

void client_send_message(int sockfd) {
    char *buffer = calloc(MAX_MESSAGE_SIZE, sizeof(char));
    printf("Please enter the message: ");
    fgets(buffer, MAX_MESSAGE_SIZE, stdin);

    size_t message_size = (int) strlen(buffer);

    if (write(sockfd, &message_size, sizeof(int)) <= 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }

    if (write(sockfd, buffer, message_size) <= 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }


}

void client_get_response(int sockfd) {
    int message_size = 0;
    if (read(sockfd, &message_size, HEADER_SIZE) <= 0) {
        PERROR_AND_EXIT("ERROR reading message size");
    }

    printf("message_size = %d\n", message_size);

    char *buffer = calloc(message_size, sizeof(char));

    if (read(sockfd, buffer, message_size) <= 0) {
        PERROR_AND_EXIT("ERROR reading message")
    }

    printf("message = %s\n", buffer);

}


int main(int argc, char *argv[]) {
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *user_name;

    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

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

    /* Create a socket point */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

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

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    client_send_message(sockfd);

    client_get_response(sockfd);


    return 0;
}