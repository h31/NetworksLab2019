//
// Created by Malik Hiraev on 10/11/2019.
//

#ifndef NETWORKSLAB2019_PRINTER_H
#define NETWORKSLAB2019_PRINTER_H

#include "server.h"

void print_client_name_and_address(struct Client *client);

void print_client_disconnected(struct Client *client);

void print_client_connected(struct Client *client);

#endif //NETWORKSLAB2019_PRINTER_H
