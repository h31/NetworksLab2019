#ifndef CLIENT_SERVER_HEADER_H
#define CLIENT_SERVER_HEADER_H

#include <time.h>

typedef struct Client {
    int fd;
    int flag;
    char *name;
    struct Client *next;
} ClientChain;

ClientChain *client_init(int fd) {
    ClientChain *temp = (ClientChain *) malloc(sizeof(ClientChain));
    temp->fd = fd;
    temp->flag = 0;
    temp->next = NULL;
    return temp;
}

ClientChain* get_client(int fd, ClientChain *root) {
    ClientChain *temp = root;
    while (temp->next != NULL) {
        if (temp->fd == fd) {
            return temp;
        }
        temp = temp->next;
    }
    if (temp->fd == fd) {
        return temp;
    }
}

void make_str_without_line_break(char *str) {
    for (int i = 0; i < (int) strlen(str); i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}

int readn(int fd, char *buffer, int len) {
    int read_size = 0;
    int res;
    while (read_size < len) {
        res = read(fd, buffer + read_size, len);
        if (res == 0) {
            read_size = res;
            break;
        }
        if (res < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        len -= res;
        read_size += res;
    }
    return read_size;
}

char *get_time() {
    time_t timer = time(NULL);
    struct tm *t;
    char tmp[6];
    char *str;
    t = localtime(&timer);
    bzero(tmp, 6);
    strftime(tmp, 6, "%H:%M", t);
    str = (char *) malloc(sizeof(tmp));
    strcpy(str, tmp);
    return str;
}

#endif //SERVER_H