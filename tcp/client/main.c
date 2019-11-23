#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include "../header.h"

void send_msg(void *arg);

void get_msg(void *arg);

void init_socket();

void init_connection(uint16_t portno, struct hostent *server);

void exit_client();

int sockfd;

int main(int argc, char *argv[]) {
    uint16_t portno;
    struct hostent *server;
    char *name;
    size_t n;
    uint32_t buff_size;
    struct pollfd fdreed;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    if (signal(SIGINT, exit_client) == SIG_ERR) {
        perror("ERROR on sigint_handler");
        exit(1);
    }

    portno = (uint16_t) atoi(argv[2]);

    init_socket();

    init_connection(portno, server);

    //get name from console
    while (1) {
        name = NULL;
        n = 0;
        printf("Enter your name (max 20 symbols):");
        buff_size = getline(&name, &n, stdin);
        if (buff_size <= 21) break;
        else printf("Invalid name, too much symbols\n");
    }
    make_str_without_line_break(name);
    printf("name: %s\n", name);

    //send name size
    if (write(sockfd, &buff_size, sizeof(int)) < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    //send name
    if (write(sockfd, name, buff_size) < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    printf("Write /exit to exit chat\n\n");

    fdreed.fd = sockfd;
    fdreed.events = POLLIN;

    pthread_t tid_send;
    if (pthread_create(&tid_send, NULL, (void *) send_msg, &sockfd) != 0) {
        printf("thread has not created\n");
        exit(1);
    }

    pthread_t tid_get;
    if (pthread_create(&tid_get, NULL, (void *) get_msg, &fdreed) != 0) {
        printf("thread has not created\n");
    }

    pthread_join(tid_send, NULL);
    pthread_join(tid_get, NULL);
    return 0;
}

//initializing client nonblock socket
void init_socket() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        perror("ERROR making socket nonblock");
        exit(1);
    }
}

//initializing connection
void init_connection(uint16_t portno, struct hostent *server) {
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    while (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        if (errno != EINPROGRESS) {
            perror("ERROR on connection");
            exit(1);
        }
    }
}

//thread function for sending
void send_msg(void *arg) {
    int sock = *(int *) arg;
    char *buffer;
    uint32_t buff_size;
    size_t n;

    while (1) {
        while (1) {
            buffer = NULL;
            n = 0;
            printf(">");
            buff_size = getline(&buffer, &n, stdin);
            if(buff_size > 1) break;
        }
        make_str_without_line_break(buffer);

        //send message size
        if (write(sock, &buff_size, sizeof(int)) < 0) {
            if (errno != EWOULDBLOCK) {
                perror("ERROR writing to socket");
                break;
            }
        }

        //send message
        if (write(sock, buffer, buff_size) < 0) {
            if (errno != EWOULDBLOCK) {
                perror("ERROR writing to socket");
                break;
            }
        }

        make_str_without_line_break(buffer);
        if (strcmp(buffer, "/exit") == 0) {
            printf("Goodbye\n");
            exit_client();
        }
    }

    exit_client();
}

//thread function for reading
void get_msg(void *arg) {
    struct pollfd fdreed = *(struct pollfd *) arg;
    int check;
    char *buffer;
    uint32_t buff_size;
    while (1) {
        check = poll(&fdreed, 1, -1);
        if (check < 0) {
            perror("ERROR on poll");
            exit(1);
        }

        if (fdreed.revents == 0) {
            continue;
        }

        if (fdreed.revents != POLLIN) {
            printf("ERROR wrong revents");
            exit(1);
        }

        //get size of message
        buff_size = 0;
        check = read(fdreed.fd, &buff_size, sizeof(int));
        if (check < 0) {
            if (errno != EWOULDBLOCK) {
                perror("ERROR reading size from socket");
                exit_client();
            }
        }
        if (check == 0) {
            printf("\rServer shutdown\n");
            exit_client();
        }

        //waiting for message income
        check = poll(&fdreed, 1, -1);
        if (check < 0) {
            perror("ERROR on poll");
            exit(1);
        }
        if (fdreed.revents != POLLIN) {
            printf("ERROR wrong revents");
            exit(1);
        }

        //get message
        buffer = (char *) malloc(buff_size);
        if (readn(fdreed.fd, buffer, buff_size) < 0) {
            if (errno != EWOULDBLOCK) {
                perror("ERROR reading msg from socket");
                exit_client();
            }
        }
        printf("\r");
        printf("%s", buffer);

        printf(">");
        fflush(stdout);
        free(buffer);
    }
}

void exit_client() {
    close(sockfd);
    printf("\n");
    exit(EXIT_SUCCESS);
}