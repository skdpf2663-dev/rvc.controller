#include "actuator.h"
void Actuator_SetPort(const ActuatorPort *port) { (void)port; }
void CommandDirection(Direction d) { (void)d; }
void CommandCleanerMode(CleanMode c) { (void)c; }
void IssueActuatorCommands(Direction d, CleanMode c) { (void)d; (void)c; }
