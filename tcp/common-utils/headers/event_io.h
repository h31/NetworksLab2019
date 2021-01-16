//
// Created by Pashnik on 01/12/2019.
//

#include <stdbool.h>
#include "common.h"

#ifndef SERVER_EVENT_IO_H
#define SERVER_EVENT_IO_H

bool read_message_header(Client *client);

bool read_name_header(Client *client);

bool read_message_body(Client *client);

bool read_name_body(Client *client);

typedef struct Helper {
    char *message;
    bool exit_code;
} Helper;

#endif //SERVER_EVENT_IO_H
