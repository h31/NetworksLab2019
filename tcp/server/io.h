#ifndef SERVER_READER_H
#define SERVER_READER_H

#include "../common-utils/headers/common.h"

#define SUCCESS 1
#define FAILURE 0

char *read_clientname(int fd, char *name_bf);

void send_message(Client *client, char *message);

char *read_message(Client *client, char *info_bf);

#endif //SERVER_READER_H
