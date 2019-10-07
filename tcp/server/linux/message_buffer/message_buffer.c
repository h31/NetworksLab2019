#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "message_buffer.h"

static pthread_mutex_t message_buffer_mutex;
char *buffer[BUFFER_SIZE];

static int head_pointer = 0;
static int tail_pointer = 0;


int increase_pointer(int pointer) {
    return ((pointer + 1) == BUFFER_SIZE) ? 0 : ++pointer;
}


int message_buffer_put(char* message) {
    int result = 1;

    pthread_mutex_lock(&message_buffer_mutex);

    if (buffer[tail_pointer] != NULL) {
        result = -1;
    } else {
        free(buffer[tail_pointer]);
        buffer[tail_pointer] = (char *) malloc(strlen(message) + 1);
        strcpy(buffer[tail_pointer], message);

        tail_pointer = increase_pointer(tail_pointer);
    }

    pthread_mutex_unlock(&message_buffer_mutex);

    return result;
}


char *message_buffer_poll(void) {
    pthread_mutex_lock(&message_buffer_mutex);

    char* result = NULL;

    if (buffer[head_pointer] != NULL) {
        result = (char*) malloc(strlen(buffer[head_pointer]) + 1);
        strcpy(result, buffer[head_pointer]);

        free(buffer[head_pointer]);
        head_pointer = increase_pointer(head_pointer);
    }

    pthread_mutex_unlock(&message_buffer_mutex);

    return result;
}


void message_buffer_init_mutex(void) {
    pthread_mutex_init(&message_buffer_mutex, NULL);
}