//
// Created by Malik Hiraev on 10/11/2019.
//

#ifndef NETWORKSLAB2019_CLIENTS_HANDLER_H
#define NETWORKSLAB2019_CLIENTS_HANDLER_H

#include "../common/common.h"
#include "server.h"

struct Client *get_first_client();

unsigned get_num_of_clients();

void accept_client(struct Client *new_client);

void remove_client(struct Client *dead_client);

#endif //NETWORKSLAB2019_CLIENTS_HANDLER_H
