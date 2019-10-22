#ifndef SERVER_ERRORS_H
#define SERVER_ERRORS_H

#include "stdio.h"
#include <stdlib.h>

#define WRONG_PORT_FORMAT "can't parse port from argv"
#define WRONG_ARGS_NUMBER "there are more than one arg"

void raise_error(char *error);

#endif //SERVER_ERRORS_H
