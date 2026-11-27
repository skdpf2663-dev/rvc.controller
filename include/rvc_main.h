/* SD module Main (SA Process 0 / Process 2 tick cycle).
 * The C main() in src/main.c only adapts stdin/stdout to these functions. */
#ifndef RVC_MAIN_H
#define RVC_MAIN_H

#include "actuator.h"
#include "capture_sensor_values.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Supplies the samples serviced in the next loop.
 * Returns false when there is no more input (the run ends). */
typedef bool (*SampleSource)(SensorSamples *samples, void *context);

/* Starts automatic cleaning: clears the sensor store and tick count,
 * connects the output port, and initializes the Controller. */
void RvcMain_Start(const ActuatorPort *port);

/* One loop: GenerateTick, CaptureSensorValues (if samples != NULL),
 * then Controller once. */
void RvcMain_Cycle(const SensorSamples *samples);

/* Repeats RvcMain_Cycle while the source gives input and fewer than
 * max_ticks ticks have run. Returns the number of ticks executed. */
unsigned long RvcMain_Run(const ActuatorPort *port, SampleSource source,
                          void *context, unsigned long max_ticks);

#ifdef __cplusplus
}
#endif

#endif /* RVC_MAIN_H */
