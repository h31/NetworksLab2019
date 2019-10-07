
#ifndef SERVER_LINUX_MESSAGE_BUFFER_H
#define SERVER_LINUX_MESSAGE_BUFFER_H

#include <pthread.h>

#define BUFFER_SIZE 1024

int message_buffer_put(char *message);

char *message_buffer_poll(void);

void message_buffer_init_mutex(void);

#endif //SERVER_LINUX_MESSAGE_BUFFER_H
