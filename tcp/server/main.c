#include <stdlib.h>
#include "stdio.h"

#include "../common-utils/headers/common.h"

/*
 * author Nikonov Pavel { <github.com/Pashnik> }
 */

void start_server(const uint16_t *port);

int main(int argc, char *argv[]) {
    uint16_t port = exclude_port(argc, argv);
    start_server(&port);
    return 0;
}
