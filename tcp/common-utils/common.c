#include "headers/common.h"
#include "headers/errors.h"

#define DECIMAL 10

/*
 * By the returned number of 'atoi' function it is impossible to determine
 * whether the string contains the correct number or not, because in
 * cases when argv = 0 and in error cases it will return 0.
 */
int exclude_port(int argc, char *argv[]) {
    if (argc != 2) raise_error(WRONG_ARGS_NUMBER);
    char *p;
    int port = (int) strtol(argv[1], &p, DECIMAL);
    if (port <= 0) raise_error(WRONG_PORT_FORMAT);
    return port;
}