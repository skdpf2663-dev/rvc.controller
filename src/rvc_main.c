#include "rvc_main.h"
void RvcMain_Start(const ActuatorPort *port) { (void)port; }
void RvcMain_Cycle(const SensorSamples *samples) { (void)samples; }
unsigned long RvcMain_Run(const ActuatorPort *port, SampleSource source, void *context, unsigned long max_ticks) { (void)port; (void)source; (void)context; (void)max_ticks; return 0; }
