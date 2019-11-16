#include "common.h"

/* readn - read exactly n bytes */
int readn(int fd, char *bp, size_t len) {
    size_t cnt;
    ssize_t rc;
    cnt = len;
    while (cnt > 0) {
        rc = recv(fd, bp, cnt, 0);
        if (rc < 0) {
            if (errno == EINTR)    /* interrupted? */
                continue;            /* restart the read */
            return -1;                /* return error */
        }
        if (rc == 0)                /* EOF? */
            return (int) (len - cnt);        /* return short count */
        bp += rc;
        cnt -= rc;
    }
    return (int) len;
}

void replace(char *buff, size_t size, char old, char new) {
    for (unsigned i = 0; i < size; ++i) {
        if (buff[i] == old) buff[i] = new;
    }
}
