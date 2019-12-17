#include "headers/errors.h"

void raise_error(char *error) {
    perror(error);
    exit(1);
}