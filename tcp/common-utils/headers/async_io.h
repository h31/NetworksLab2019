#ifndef CLIENT_ASYNC_IO_H
#define CLIENT_ASYNC_IO_H

#include "common.h"

void read_header(Client *client);

void read_message(Client *client);

#endif //CLIENT_ASYNC_IO_H
