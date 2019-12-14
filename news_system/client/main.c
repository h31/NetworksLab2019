#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <signal.h>

#include "./user_choice_handler/functions.h"
#include "./window_manager/window_manager.h"
#include "./window_manager/user_choice.h"



void check_number_of_args(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
}


void fill_server_info_(struct sockaddr_in* serv_addr, struct hostent *server, uint16_t port) {
    bzero((char *) serv_addr, sizeof(*serv_addr));
    serv_addr -> sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &(serv_addr->sin_addr.s_addr), (size_t) server->h_length);
    serv_addr -> sin_port = htons(port);
}


void stop_client_(int sockfd, char* ret_text) {
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    if (ret_text != NULL) {
        printf("%s\n", ret_text);
    }
    exit(0);
}


int main(int argc, char* argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    check_number_of_args(argc, argv);

    portno = (uint16_t) atoi(argv[2]);


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

    fill_server_info_(&serv_addr, server, portno);


    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);

    int user_choice;
    while (1) {
        user_choice = get_user_choice();

        switch (user_choice) {
            case GET_LIST_OF_TOPICS:
                if (handle_get_list_of_topics_user_choice(sockfd) <= 0) {
                    stop_client_(sockfd, "ERROR server disconnected");
                }
                break;

            case GET_LIST_OF_NEWS:
                if (handle_get_list_of_news_user_choice(sockfd) <= 0) {
                    stop_client_(sockfd,"ERROR server disconnected");
                }
                break;

            case GET_NEWS:
                if (handle_get_news_user_choice(sockfd) <= 0) {
                    stop_client_(sockfd,"ERROR server disconnected");
                }
                break;

            case ADD_TOPIC:
                if (handle_add_topic_user_choice(sockfd) <= 0) {
                    stop_client_(sockfd, "ERROR server disconnected");
                }
                break;

            case ADD_NEWS:
                if (handle_add_news_user_choice(sockfd) <= 0) {
                    stop_client_(sockfd, "ERROR server disconnected");
                }
                break;

            case QUIT:
                stop_client_(sockfd, NULL);
            default:
                break;
        }
    }

}