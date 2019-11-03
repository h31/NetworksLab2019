#include "../common-utils/headers/common.h"
#include <ntsid.h>
#include <memory.h>
#include <unistd.h>
#include "../common-utils/headers/errors.h"

char *read_username(const int *fd, char *name_bf) {
    ssize_t n;
    memset(name_bf, 0, sizeof(char));
    n = read(*fd, name_bf, 10);
    if (n == 0) raise_error(SOCKET_READ_ERROR);
    return name_bf;
}
