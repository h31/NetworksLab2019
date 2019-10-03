#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "../../common.h"

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFF_SIZE];
    ///////////////////////////////////////////////////////
    ///////////////// parsing arguments ///////////////////
    ///////////////////////////////////////////////////////
    if (argc < 3) {
        fprintf(stderr, "Illegal amount of arguments < 2.\n"
                        "-s(server) and -p(port) must be passed\n");
        exit(1);
    }

    int opt;
    while ((opt = getopt(argc, argv, "s:p:")) != -1) {
        switch (opt) {
            case 's':
                server = gethostbyname(optarg);
                if (server == NULL) {
                    fprintf(stderr, "ERROR, no such host\n");
                    exit(0);
                }
                break;
            case 'p':
                portno = (uint16_t) atoi(optarg);
                break;
            default:
                /* unrecognised opt ... add your error condition */
                printf("Unrecognized opt = %s", optarg);
                break;
        }
    }

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        PERROR_AND_EXIT("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
        < 0) {
        PERROR_AND_EXIT("ERROR connecting");
    }

    /* Now ask for a message from the user, this message
     * will be read by server
     */
    printf("Please enter the message: ");
    bzero(buffer, BUFF_SIZE);
    fgets(buffer, BUFF_SIZE - 1, stdin);

    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        PERROR_AND_EXIT("ERROR writing to socket");
    }

    /* Now read server response */
    bzero(buffer, BUFF_SIZE);

    if (read(sockfd, buffer, BUFF_SIZE - 1) < 0) {
        PERROR_AND_EXIT("ERROR reading from socket");
    }

    printf("%s\n", buffer);
    return 0;
}
