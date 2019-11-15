//
// Created by Malik Hiraev on 10/11/2019.
//

#ifndef NETWORKSLAB2019_PRINTER_H
#define NETWORKSLAB2019_PRINTER_H

#include <stdio.h>
#include <netdb.h>

#include "client.h"

void print_client_name_and_address(Client *client);

void print_client_disconnected(Client *client);

void print_client_connected(Client *client);

#endif //NETWORKSLAB2019_PRINTER_H
