/* Controller (SA Process 2.2.2 Control Cleaning Behavior = DFD Level 4 FSM).
 * Implements the pseudocode of process_specification.md Process 2.2.2 and the
 * transition table of artifacts/SA/DFD/dfd_level_4_fsm.md. FSM state, counters
 * and dust_during_turn are hidden in this file (information hiding). */
#include "controller.h"

#include "actuator.h"
#include "sensor_store.h"

#define TURN_TICKS 3    /* PR-10: a turn lasts 3 ticks */
#define CLEANUP_TICKS 3 /* PR-11: one clean_up activation lasts at most 3 ticks */

static ControllerStatus ctl;

static void set_commands(Direction direction, CleanMode clean)
{
    ctl.direction_command = direction;
    ctl.clean_command = clean;
}

static void start_turn(ControllerState turn_state, bool dust)
{
    ctl.state = turn_state;
    ctl.turn_count = 1;
    ctl.dust_during_turn = dust;
    set_commands(turn_state == STATE_TURN_LEFT ? DIR_TURN_LEFT : DIR_TURN_RIGHT, CLEAN_OFF);
}

static void start_cleanup(ControllerState cleanup_state)
{
    ctl.state = cleanup_state;
    ctl.cleanup_count = 1;
    set_commands(cleanup_state == STATE_CLEANUP_FORWARD ? DIR_FORWARD : DIR_STOP, CLEAN_UP);
}

static void enter_backward(void)
{
    ctl.state = STATE_BACKWARD;
    set_commands(DIR_BACKWARD, CLEAN_OFF);
}

static void enter_forward(void)
{
    ctl.state = STATE_FORWARD;
    set_commands(DIR_FORWARD, CLEAN_ON);
}

/* Obstacle avoidance shared by Forward and the end of clean_up:
 * left turn has priority (PR-9); with every side blocked the RVC backs up (PR-4).
 * Called only when the front is blocked. */
static void avoid_front_obstacle(const SensorValues *in, bool dust_for_turn)
{
    if (!in->left) {
        start_turn(STATE_TURN_LEFT, dust_for_turn);
    } else if (!in->right) {
        start_turn(STATE_TURN_RIGHT, dust_for_turn);
    } else {
        enter_backward();
    }
}

static void step_forward(const SensorValues *in)
{
    if (in->front) {
        /* Interpretation 5: a front obstacle is handled before dust. */
        avoid_front_obstacle(in, in->dust);
    } else if (in->dust) {
        start_cleanup(STATE_CLEANUP_FORWARD);
    } else {
        enter_forward();
    }
}

static void step_turn(void)
{
    if (ctl.turn_count < TURN_TICKS) {
        ctl.turn_count++;
        set_commands(ctl.state == STATE_TURN_RIGHT ? DIR_TURN_RIGHT : DIR_TURN_LEFT, CLEAN_OFF);
    } else if (ctl.dust_during_turn) {
        ctl.dust_during_turn = false;
        start_cleanup(STATE_CLEANUP_FORWARD);
    } else {
        ctl.dust_during_turn = false;
        enter_forward();
    }
}

static void step_backward(const SensorValues *in)
{
    /* Interpretation 9: a clear side is handled before dust. */
    if (!in->left) {
        start_turn(STATE_TURN_LEFT, in->dust);
    } else if (!in->right) {
        start_turn(STATE_TURN_RIGHT, in->dust);
    } else if (in->dust) {
        start_cleanup(STATE_CLEANUP_BACKWARD_PAUSE);
    } else {
        enter_backward();
    }
}

static void step_backward_pause(const SensorValues *in)
{
    if (ctl.cleanup_count == CLEANUP_TICKS || !in->dust) {
        enter_backward();
    } else {
        ctl.cleanup_count++;
        set_commands(DIR_STOP, CLEAN_UP);
    }
}

static void step_cleanup(const SensorValues *in)
{
    const bool activation_ended = (ctl.cleanup_count == CLEANUP_TICKS || !in->dust);
    const bool is_forward = (ctl.state == STATE_CLEANUP_FORWARD);

    if (!activation_ended) {
        ctl.cleanup_count++;
        if (is_forward && in->front) {
            /* PR-15: front obstacle during clean_up stops in place, same activation. */
            ctl.state = STATE_CLEANUP_STOPPED;
        }
        set_commands(ctl.state == STATE_CLEANUP_FORWARD ? DIR_FORWARD : DIR_STOP, CLEAN_UP);
    } else if (in->dust) {
        /* Next activation: stopped if already stopped or the front is blocked. */
        start_cleanup((!is_forward || in->front) ? STATE_CLEANUP_STOPPED : STATE_CLEANUP_FORWARD);
    } else if (in->front) {
        avoid_front_obstacle(in, false);
    } else {
        enter_forward();
    }
}

void Controller_Init(void)
{
    ctl.turn_count = 0;
    ctl.cleanup_count = 0;
    ctl.dust_during_turn = false;
    enter_forward();
    /* Automatic cleaning begins with forward and clean_on. */
    IssueActuatorCommands(ctl.direction_command, ctl.clean_command);
}

void Controller(void)
{
    const SensorValues in = ReadLatestSensorValues();

    if ((ctl.state == STATE_TURN_LEFT || ctl.state == STATE_TURN_RIGHT) && in.dust) {
        ctl.dust_during_turn = true;
    }

    switch (ctl.state) {
    case STATE_FORWARD:
        step_forward(&in);
        break;
    case STATE_TURN_LEFT:
    case STATE_TURN_RIGHT:
        step_turn();
        break;
    case STATE_BACKWARD:
        step_backward(&in);
        break;
    case STATE_CLEANUP_BACKWARD_PAUSE:
        step_backward_pause(&in);
        break;
    case STATE_CLEANUP_FORWARD:
    case STATE_CLEANUP_STOPPED:
        step_cleanup(&in);
        break;
    default:
        /* Unreachable: the state is always one of the seven FSM states. */
        enter_forward();
        break;
    }

    IssueActuatorCommands(ctl.direction_command, ctl.clean_command);
}

ControllerStatus Controller_GetStatus(void)
{
    return ctl;
}
