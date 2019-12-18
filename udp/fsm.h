#ifndef FSM
#define FSM

#define STATE_RESET 1
#define STATE_STANDBY 2
#define STATE_RECEIVE 3
#define STATE_SEND 4
#define STATE_WAIT 5
#define STATE_FINISHED 6

#define OPERATION_DONE 1
#define OPERATION_ABANDONED 2
#define OPERATION_FAILED 3

void server_fsm(int *state, int *operation);
void client_fsm(int *state, int *operation);

#endif