/* SD module Controller (SA Process 2.2.2, DFD Level 4 FSM). */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "rvc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATE_FORWARD = 0,
    STATE_CLEANUP_FORWARD,
    STATE_CLEANUP_STOPPED,
    STATE_TURN_LEFT,
    STATE_TURN_RIGHT,
    STATE_BACKWARD,
    STATE_CLEANUP_BACKWARD_PAUSE
} ControllerState;

/* Read-only snapshot of the hidden controller state, for inspection only. */
typedef struct {
    ControllerState state;
    int turn_count;
    int cleanup_count;
    bool dust_during_turn;
    Direction direction_command;
    CleanMode clean_command;
} ControllerStatus;

/* Begin automatic cleaning: state Forward, issues forward and clean_on. */
void Controller_Init(void);
/* Executes one FSM transition. Called once per Tick. */
void Controller(void);
ControllerStatus Controller_GetStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* CONTROLLER_H */
