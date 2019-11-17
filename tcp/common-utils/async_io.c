#include "headers/common.h"
#include "unistd.h"

int read_message_size(Client *client) {
    ssize_t n;
    size_t message_size = 0;

    /* firstly read message size, and then message */
    n = read(client->id, (void *) &message_size, HEADER_SIZE);
}

