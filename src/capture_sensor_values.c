/* CaptureSensorValues (SA Process 1): services arrived samples by calling only
 * the update modules whose new samples are available. */
#include "capture_sensor_values.h"
#include "sensor_store.h"

#include <stddef.h>

void CaptureSensorValues(const SensorSamples *samples)
{
    if (samples == NULL) {
        return;
    }
    if (samples->front_arrived) {
        UpdateFrontSensor(samples->front);
    }
    if (samples->left_arrived) {
        UpdateLeftSensor(samples->left);
    }
    if (samples->right_arrived) {
        UpdateRightSensor(samples->right);
    }
    if (samples->dust_arrived) {
        UpdateDustSensor(samples->dust);
    }
}
