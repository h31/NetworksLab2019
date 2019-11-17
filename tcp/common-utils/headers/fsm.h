#ifndef CLIENT_FSM_H
#define CLIENT_FSM_H

enum FSM {
    FHEADER_SIZE, FHEADER, FMSG_SIZE, FMSG
};

typedef enum FSM State;

#endif //CLIENT_FSM_H
