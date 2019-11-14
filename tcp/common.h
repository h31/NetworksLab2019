//
// Created by danila on 10/10/2019.
//
#include <poll.h>

#define CL_STAT_ON  0
#define CL_STAT_OFF 1

#define VERBOSE_MUTEX_LOCK(mutex_ptr, place)  \
        pthread_mutex_lock(mutex_ptr);\
        printf("\n------------------------\nMutex locked before ");\
        printf(place);\
        printf("\n------------------------\n");

#define VERBOSE_MUTEX_UNLOCK(mutex_ptr, place)\
        pthread_mutex_unlock(mutex_ptr);\
        printf("\n------------------------\nMutex unlocked after ");\
        printf(place);\
        printf("\n------------------------\n");

#define PRETTY_MARK_AND_DO(code)    \
    printf("\n-----------------------------\n");\
    code\
    printf("\n-----------------------------\n");

struct Client {
    int sockfd;
    char *name;
    int status;
    struct sockaddr_in *sockaddr;
    struct Client *next_client;
    struct Client *prev_client;
} typedef Client;

struct Message {
    char *buffer;
    int size;
    int err;
} typedef Message;

struct PollfdList {
    struct pollfd *pollfds;
    int size;
    int max_size;
} typedef PollfdList;

Message *get_new_message(char *buffer) {
    Message *message = calloc(1, sizeof(Message));
    message->size = strlen(buffer);
    message->buffer = buffer;
    return message;
}

Client *get_new_client_empty() {
    Client *new_client = calloc(1, sizeof(Client));
    new_client->next_client = NULL;
    new_client->status = CL_STAT_OFF;

    return new_client;
}

#define PERROR_AND_EXIT(message){\
    perror(message);\
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

int readN(int fd, char *buffer, int msg_size) {
    int curr_buff_offset = 0;
    int curr_readed_bytes = 0;
    while (curr_buff_offset < msg_size) {
        curr_readed_bytes = read(
                fd,
                buffer + curr_buff_offset,
                msg_size - curr_buff_offset
        );
        if (curr_readed_bytes == 0) {
            curr_buff_offset = curr_readed_bytes;
            break;
        } else if (curr_readed_bytes < 0) {
            PERROR_AND_EXIT("ERROR reading from socket")
        } else {
            curr_buff_offset += curr_readed_bytes;
        }
    }
    return curr_buff_offset;
}
