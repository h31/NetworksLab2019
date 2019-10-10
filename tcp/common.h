//
// Created by danila on 10/10/2019.
//

struct Client {
    int sockfd;
    char *name;
    int status;
    pthread_t thread;

} typedef Client;

struct Message {
    char *buffer;
    int size;
} typedef Message;

Message *NewMessage(char *buffer) {
    Message *message = calloc(1, sizeof(Message));
    message->size = strlen(buffer);
    message->buffer = buffer;
    return message;
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