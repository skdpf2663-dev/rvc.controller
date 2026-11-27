/* Main (SD root module): coordinates tick generation, sensor servicing, and
 * one Controller execution per tick (structure_chart.md section 3). */
#include "rvc_main.h"

#include <stddef.h>

#include "controller.h"
#include "generate_tick.h"
#include "sensor_store.h"

void RvcMain_Start(const ActuatorPort *port)
{
    SensorStore_Reset();
    GenerateTick_Reset();
    Actuator_SetPort(port);
    Controller_Init();
}

void RvcMain_Cycle(const SensorSamples *samples)
{
    GenerateTick();
    /* Samples that arrived are applied before the decision reads them. */
    CaptureSensorValues(samples);
    Controller();
}

unsigned long RvcMain_Run(const ActuatorPort *port, SampleSource source,
                          void *context, unsigned long max_ticks)
{
    unsigned long executed = 0UL;
    SensorSamples samples;

    RvcMain_Start(port);
    if (source == NULL) {
        return 0UL;
    }
    while (executed < max_ticks && source(&samples, context)) {
        RvcMain_Cycle(&samples);
        executed++;
    }
    return executed;
}
