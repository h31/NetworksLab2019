#ifndef CLIENT_SERVER_HEADER_H
#define CLIENT_SERVER_HEADER_H

typedef struct Client {
    int sock;
    char name[25];
    struct Client *prev;
    struct Client *next;
} ClientChain;

ClientChain *chain_init(int sock){
    ClientChain *temp = (ClientChain *) malloc(sizeof(ClientChain));
    temp->sock = sock;
    strncpy(temp->name, "null", 5);
    temp->prev = NULL;
    temp->next = NULL;
    return temp;
}

void make_str(char *str) {
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
        if (res < 0){
            perror("ERROR reading from socket");
            exit(1);
        }
        read_size += res;
    }
    return read_size;
}


#endif //SERVER_H
