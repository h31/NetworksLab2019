#ifndef CLIENT_FSM_H
#define CLIENT_FSM_H

enum FSM {
    ST_NAME_HEADER, ST_NAME, ST_MESSAGE_HEADER, ST_MESSAGE
};

typedef enum FSM State;

State next(State state);

#endif //CLIENT_FSM_H
