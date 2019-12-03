#include "headers/common.h"

#define ACCEPTOR_POS 0
#define TIMEOUT -1

static poll_descriptor descriptors[MAX_CLIENTS];
static unsigned size = 1;

void add_to_descriptors(int fd) {
    descriptors[size].fd = fd;
    descriptors[size].events = POLL_IN;
    size++;
}

void add_acceptor_to_descriptors(int fd) {
    descriptors[ACCEPTOR_POS].fd = fd;
    descriptors[ACCEPTOR_POS].events = POLL_IN;
}

poll_descriptor get_acceptor() {
    return descriptors[ACCEPTOR_POS];
}

void zero_revents(int index){
    descriptors[index].revents = 0;
}

int poll_() {
    return poll(descriptors, size, TIMEOUT);
}

poll_descriptor getn_from_descriptor(int n) {
    return descriptors[n];
}

int get_size() {
    return size;
}

int accept_() {
    return accept(get_acceptor().fd, NULL, NULL);
}