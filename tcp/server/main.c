#include <stdlib.h>
#include "stdio.h"

#include "../common-utils/headers/common.h"

#include "../common-utils/headers/list.h"

/*
 * author Nikonov Pavel { <github.com/Pashnik> }
 */

void start_server(const uint16_t *port);

int main(int argc, char *argv[]) {
    uint16_t port = exclude_servport(argc, argv);
    start_server(&port);
    return 0;
}
