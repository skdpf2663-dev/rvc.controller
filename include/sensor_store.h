/* D1 Latest Sensor Values and the modules that access it.
 * SD modules: UpdateFrontSensor, UpdateLeftSensor, UpdateRightSensor,
 * UpdateDustSensor, ReadLatestSensorValues (SA 1.1-1.4, 2.2.1).
 * The store itself is hidden inside sensor_store.c. */
#ifndef SENSOR_STORE_H
#define SENSOR_STORE_H

#include "rvc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Sets every stored value to false (initial value until the first sample). */
void SensorStore_Reset(void);

void UpdateFrontSensor(bool front_sensor_input);
void UpdateLeftSensor(bool left_sensor_input);
void UpdateRightSensor(bool right_sensor_input);
void UpdateDustSensor(bool dust_sensor_input);

/* Returns the four stored booleans without changing them. */
SensorValues ReadLatestSensorValues(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_STORE_H */
