/* SD module CaptureSensorValues (SA Process 1). */
#ifndef CAPTURE_SENSOR_VALUES_H
#define CAPTURE_SENSOR_VALUES_H

#include "rvc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Sensor samples that have arrived since the last servicing.
 * xxx_arrived tells whether a new sample of that sensor is available. */
typedef struct {
    bool front_arrived; /* front: interrupt arrival */
    bool front;
    bool left_arrived;  /* left: periodic arrival   */
    bool left;
    bool right_arrived; /* right: periodic arrival  */
    bool right;
    bool dust_arrived;  /* dust: periodic arrival   */
    bool dust;
} SensorSamples;

/* Calls only the update modules whose new samples are available.
 * A NULL pointer means no sample arrived. */
void CaptureSensorValues(const SensorSamples *samples);

#ifdef __cplusplus
}
#endif

#endif /* CAPTURE_SENSOR_VALUES_H */
