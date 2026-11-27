#include "controller.h"
void Controller_Init(void) {}
void Controller(void) {}
ControllerStatus Controller_GetStatus(void) { ControllerStatus s = {STATE_FORWARD, 0, 0, false, DIR_FORWARD, CLEAN_ON}; return s; }
