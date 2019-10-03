#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "../../common.h"

char *string_concat(char *line1, char *line2) {
    size_t line_len_1 = strlen(line1);
    size_t line_len_2 = strlen(line2);

    char *totalLine = malloc(line_len_1 + line_len_2 + 1);
    if (!totalLine) abort();

    memcpy(totalLine, line1, line_len_1);
    memcpy(totalLine + line_len_1, line2, line_len_2);
    totalLine[line_len_1 + line_len_2] = '\0';

    return totalLine;

}

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *user_name = NULL;

    char buffer_message[BUFF_SIZE];
    ///////////////////////////////////////////////////////
    ///////////////// parsing arguments ///////////////////
    ///////////////////////////////////////////////////////
    if (argc < 3) {
        fprintf(stderr, "Illegal amount of arguments < 2.\n"
                        "-s(server) and -p(port) must be passed\n");
        exit(1);
    }

    int opt;
    while ((opt = getopt(argc, argv, "s:p:n:")) != -1) {
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
            case 'n':
                user_name = strdup(optarg);
                break;
            default:
                /* unrecognised opt ... add your error condition */
                printf("Unrecognized opt = %s", optarg);
                exit(1);
        }
    }

    if (user_name == NULL) {
        PERROR_AND_EXIT("user_name = NULL\n")
    }
    user_name = string_concat(user_name, ": ");
    printf("user_name = %s\n", user_name);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        PERROR_AND_EXIT("ERROR opening socket");
    }

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
        < 0) {
        PERROR_AND_EXIT("ERROR connecting");
    }

    while (1) {
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
        serv_addr.sin_port = htons(portno);


        /* Now ask for a message from the user, this message
         * will be read by server
         */

        printf("%s", user_name);
        bzero(buffer_message, BUFF_SIZE);
        fgets(buffer_message, BUFF_SIZE - 1, stdin);

        char *buffer_full = string_concat(user_name, &buffer_message);
        if (write(sockfd, buffer_full, strlen(buffer_full)) < 0) {
            PERROR_AND_EXIT("ERROR writing to socket");
        }

        /* Now read server response */
        bzero(buffer_message, BUFF_SIZE);

        if (read(sockfd, buffer_message, BUFF_SIZE - 1) < 0) {
            PERROR_AND_EXIT("ERROR reading from socket");
        }

        printf("%s\n", buffer_message);
    }
    return 0;
}
