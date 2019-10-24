//
// Created by danila on 10/10/2019.
//

struct Client {
    int sockfd;
    char *name;
    struct sockaddr_in *sockaddr;
    pthread_t thread;
    struct Client *next_client;
    struct Client *prev_client;
} typedef Client;

struct Message {
    char *buffer;
    int size;
} typedef Message;

Message *get_new_message(char *buffer) {
    Message *message = calloc(1, sizeof(Message));
    message->size = strlen(buffer);
    message->buffer = buffer;
    return message;
}

Client *get_new_client_empty() {
    Client *new_client = calloc(1, sizeof(Client));
    new_client->next_client = NULL;

    return new_client;
}

#define PERROR_AND_EXIT(message){\
    perror("ERROR opening socket");\
    exit(1);\
}

#define ASSERT(expression, message){\
    if(!expression){\
        fprintf(stderr, message);\
        exit(0);\
    }\
}

char *str_concat(char *line1, char *line2) {
    size_t line_len_1 = strlen(line1);
    size_t line_len_2 = strlen(line2);

    char *totalLine = calloc(line_len_1 + line_len_2 + 1, sizeof(char));
    if (!totalLine) {
        abort();
    }

    memcpy(totalLine, line1, line_len_1);
    memcpy(totalLine + line_len_1, line2, line_len_2);
    totalLine[line_len_1 + line_len_2] = '\0';

    return totalLine;

}
