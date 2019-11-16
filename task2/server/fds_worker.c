//
// Created by Malik Hiraev on 10/11/2019.
//

#include "fds_worker.h"

static struct pollfd fds[MAX_CLIENTS_SIZE + 1]; // First for acceptor (so + 1)
static unsigned fds_size = 1; // 1 is minimal value when there are no any clients (for acceptor)

static unsigned get_last_index() {
    return fds_size - 1;
}

void init_fds_worker(int socket_fd) {
    memset(fds, 0, sizeof(fds));
    // Configure first structure for acceptor
    fds[0].fd = socket_fd;
    fds[0].events = POLLIN;
}

int get_acceptor_revents() {
    return fds[0].revents;
}

unsigned get_fds_size() {
    return fds_size;
}

void close_all_fds() {
    for (unsigned i = 0; i < fds_size; ++i) {
        shutdown(fds[i].fd, SHUT_RDWR);
        close(fds[i].fd);
    }
}

void remove_and_close_fd(unsigned index) {
    struct pollfd pfd = fds[index];
    shutdown(pfd.fd, SHUT_RDWR);
    close(pfd.fd);
    /**
     * Restructure to avoid empty cells in the array
     */
    unsigned last_index = get_last_index();
    bool is_not_last = index != last_index;
    if (is_not_last) {
        memccpy(&fds[index], &fds[last_index], 0, sizeof(struct pollfd));
    }
    fds_size--;
}

struct pollfd *get_pollfd(unsigned index) {
    return &fds[index];
}

int poll_inner() {
    return poll(fds, get_fds_size(), POLL_TIMEOUT);
}

int add_fd(int fd) {
    if (fds_size == MAX_CLIENTS_SIZE + 1) return -1;
    fds[fds_size].fd = fd;
    fds[fds_size++].events = POLLIN;
    return get_last_index();
}
