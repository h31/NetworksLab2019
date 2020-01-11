#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "termios.h"
#include "signal.h"
#include "time.h"

#include "../common/common.h"

pthread_mutex_t mutex;
int sockfd;

char get_char() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

struct args {
    int sock_fd;
    char *buffer;
};

void *read_func(void *input) {
    time_t now;
    struct tm time_struct;
    int status;
    int sock_fd = ((struct args *) input)->sock_fd;

    while (1) {
        int len = 0;
        status = read(sock_fd, &len, sizeof(int));
        if (status < 0) {
            perror("ERROR reading message length from socket");
            exit(1);
        }
        if (len != 0) {
            pthread_mutex_lock(&mutex);
            char *message = (char *) malloc(len * sizeof(char));
            bzero(message, len * sizeof(char));
            status = read(sock_fd, message, len);
            if (status < 0) {
                perror("ERROR reading from socket");
                exit(1);
            }
            time(&now);
            time_struct = *localtime(&now);
            printf(ANSI_COLOR_GREEN"<%02d:%02d> %s", time_struct.tm_hour,
                   time_struct.tm_min, message);
            pthread_mutex_unlock(&mutex);
        } else {
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            printf("Connection lost\n");
            exit(0);
        }
    }
}

void signal_handler() {
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    printf("Disconnected.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int status;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *username;
    char buffer[MESSAGE_LEN];
    time_t now;
    struct tm time_struct;
    signal(SIGINT, signal_handler);

    pthread_t read_thread;
    pthread_mutex_init(&mutex, NULL);

    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port username\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);
    username = argv[3];

    /* Create a socket point */
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

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    int username_len = strlen(username);
    status = write(sockfd, &username_len, sizeof(int));
    if (status < 0) {
        perror("ERROR writing username length to socket");
        exit(1);
    }

    status = write(sockfd, username, username_len);
    if (status < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    printf("Welcome to the club, %s! Press \"m\" to type a message or Ctrl+C to exit.\n", username);

    struct args *a = (struct args *) malloc(sizeof(struct args));
    a->buffer = buffer;
    a->sock_fd = sockfd;

    pthread_create(&read_thread, NULL, (void *) read_func, (void *) a);

    while (1) {
        /* Now ask for a message from the user, this message
         * will be read by server
        */
        int ch = get_char();
        while (ch != 'm') {
            ch = get_char();
        }
        pthread_mutex_lock(&mutex);
        time(&now);
        time_struct = *localtime(&now);
        printf(ANSI_COLOR_GREEN"<%02d:%02d> "ANSI_COLOR_BLUE"[%s]"ANSI_COLOR_RESET": ", time_struct.tm_hour,
               time_struct.tm_min, username);
        bzero(buffer, MESSAGE_LEN);
        fgets(buffer, MESSAGE_LEN - 1, stdin);
        int str_len = strlen(buffer);

        /* Send message to the server */
        status = write(sockfd, &str_len, sizeof(int));
        if (status < 0) {
            perror("ERROR writing length to socket");
            exit(1);
        }
        status = write(sockfd, buffer, str_len);
        if (status < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}