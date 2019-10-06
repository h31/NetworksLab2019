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

#endif //SERVER_H
