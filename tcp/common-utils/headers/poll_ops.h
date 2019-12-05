#ifndef SERVER_POLL_OPS_H
#define SERVER_POLL_OPS_H

#include "inet_utils.h"

void add_acceptor_to_descriptors(int fd);

void add_to_descriptors(int fd);

poll_descriptor get_acceptor();

void zero_revents(int index);

int poll_();

int get_size();

poll_descriptor getn_from_descriptor(int n);

int accept_();

void close_();

void close_n(int n);

#endif //SERVER_POLL_OPS_H
