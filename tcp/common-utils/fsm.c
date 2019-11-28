#include "headers/fsm.h"
#include "headers/errors.h"

State next(State state) {
    switch (state) {
        case ST_NAME_HEADER:
            return ST_NAME;
            break;
        case ST_NAME:
            return ST_MESSAGE_HEADER;
            break;
        case ST_MESSAGE_HEADER:
            return ST_MESSAGE;
            break;
        case ST_MESSAGE:
            return ST_MESSAGE_HEADER;
            break;
        default:
            raise_error(SHOULD_NOT_BE);
            return ST_NAME_HEADER;
    }
}

