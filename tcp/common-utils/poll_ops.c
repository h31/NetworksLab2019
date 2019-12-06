#include "headers/common.h"
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define ACCEPTOR_POS 0
#define TIMEOUT -1

poll_descriptor descriptors[MAX_CLIENTS];
unsigned size = 1;

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

void zero_revents(int index) {
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

int find_n(int index) {
    for (unsigned int i = 0; i < size; ++i) {
        if (descriptors[i].fd == index) return i;
    }
    return -1;
}

void zero_all(int index) {
    descriptors[index].fd = 0;
    zero_revents(index);
    descriptors[index].events = 0;
}

void close_n(int n) {
    int idx = find_n(n);

    close(descriptors[idx].fd);
    zero_all(idx);

    if ((int) (size - 1) != idx) {
        memcpy(&descriptors[idx], &descriptors[size - 1], sizeof(poll_descriptor));
        zero_all(idx + 1);
    }
    size--;
}

void close_() {
    for (unsigned int i = 0; i < size - 1; ++i) {
        close(descriptors[i].fd);
    }
}