#include "headers/fsm.h"
#include "headers/errors.h"

State on_next(State state) {
    switch (state) {
        case FHEADER_SIZE:
            return FHEADER;
            break;
        case FHEADER:
            return FMSG_SIZE;
            break;
        case FMSG_SIZE:
            return FMSG;
            break;
        default:
            raise_error(SHOULD_NOT_BE);
    }
}

