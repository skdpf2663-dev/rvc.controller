/* IssueActuatorCommands, CommandDirection, CommandCleanerMode (SA 3, 3.1, 3.2).
 * These modules deliver the selected commands; they never choose a command. */
#include "actuator.h"

#include <stddef.h>

static const ActuatorPort *output_port = NULL;

void Actuator_SetPort(const ActuatorPort *port)
{
    output_port = port;
}

void CommandDirection(Direction direction_command)
{
    if (output_port != NULL && output_port->send_direction != NULL) {
        output_port->send_direction(direction_command);
    }
}

void CommandCleanerMode(CleanMode clean_command)
{
    if (output_port != NULL && output_port->send_clean != NULL) {
        output_port->send_clean(clean_command);
    }
}

void IssueActuatorCommands(Direction direction_command, CleanMode clean_command)
{
    CommandDirection(direction_command);
    CommandCleanerMode(clean_command);
}
