#ifndef SERVER_POLL_OPS_H
#define SERVER_POLL_OPS_H

#include "inet_utils.h"

void add(int fd);

void add_acceptor(int fd);

poll_descriptor get_acceptor();

int poll_();

int get_size();

poll_descriptor getn(int n);

int accept_();

#endif //SERVER_POLL_OPS_H
