#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

#include "threads/listening_thread/thread.h"
#include "threads/writing_thread/thread.h"
#include "main_window/main_window.h"
#include "constants.h"

void check_number_of_args(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
}


int main(int argc, char* argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    check_number_of_args(argc, argv);

    portno = (uint16_t) atoi(argv[2]);

    // Create a socket point
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    // Now connect to the server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }


    main_window_init();

    pthread_t writing_thread = create_writing_thread(sockfd, MAX_MESSAGE_LENGTH);
    pthread_join(create_listening_thread(sockfd, OUTPUT_WINDOW_HEIGHT), NULL);
    pthread_cancel(writing_thread);



    return 0;
}