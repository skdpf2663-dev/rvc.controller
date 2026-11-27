/* SD modules IssueActuatorCommands, CommandDirection, CommandCleanerMode
 * (SA Process 3, 3.1, 3.2). Hardware control is out of scope (PR-6), so the
 * motor and the cleaner are reached through an output port of two callbacks. */
#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "rvc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*send_direction)(Direction direction); /* to the motor   */
    void (*send_clean)(CleanMode clean);         /* to the cleaner */
} ActuatorPort;

/* Connects the output port. NULL disconnects it; commands are then dropped. */
void Actuator_SetPort(const ActuatorPort *port);

void CommandDirection(Direction direction_command);
void CommandCleanerMode(CleanMode clean_command);
/* Delivers both commands: CommandDirection, then CommandCleanerMode. */
void IssueActuatorCommands(Direction direction_command, CleanMode clean_command);

#ifdef __cplusplus
}
#endif

#endif /* ACTUATOR_H */
