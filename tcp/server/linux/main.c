#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include "threads/listening_thread/thread.h"
#include "threads/writing_thread/thread.h"
#include "message_buffer/message_buffer.h"


// checking number of arguments
void checkArguments(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage %s port pipe_path\n", argv[0]);
        exit(0);
    };
}


int main(int argc, char* argv[]) {

    checkArguments(argc, argv);

    const uint16_t port_number = (uint16_t)  atoi(argv[1]);

    user_info_init_mutex();

    fprintf(stdout, "server start at port: %d\n", port_number);

    int initial_socket, newsockfd;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;

    // opening init socket
    initial_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (initial_socket < 0) {
        fprintf(stderr, "ERROR opening init socket\n");
        exit(1);
    }

    /* Initialize initial_socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_number);

    /* bind the host address using bind() call.*/
    if (bind(initial_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding\n");
        exit(1);
    }

    listen(initial_socket, 5);
    clilen = sizeof(cli_addr);


    message_buffer_init_mutex();

    // сreate writing thread
    create_writing_thread();

    // accepting new connection and creating listening threads
    while(1) {
        newsockfd = accept(initial_socket, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {
            perror("ERROR on accept\n");
            exit(1);
        }

        create_listening_thread(newsockfd);
    }
}
