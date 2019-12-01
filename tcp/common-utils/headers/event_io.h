//
// Created by Pashnik on 01/12/2019.
//

#include "common.h"

#ifndef SERVER_EVENT_IO_H
#define SERVER_EVENT_IO_H

void read_message_header(Client *client);

void read_name_header(Client *client);

void read_message_body(Client *client);

void read_name_body(Client *client);

#endif //SERVER_EVENT_IO_H
