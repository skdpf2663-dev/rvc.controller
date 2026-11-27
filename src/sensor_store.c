/* D1 Latest Sensor Values (hidden store) and its access modules.
 * Traceability: SA 1.1-1.4 Update*Sensor, SA 2.2.1 ReadLatestSensorValues. */
#include "sensor_store.h"

/* File-scope store: only the five modules below can reach it (information hiding). */
static SensorValues latest = {false, false, false, false};

void SensorStore_Reset(void)
{
    latest.front = false;
    latest.left = false;
    latest.right = false;
    latest.dust = false;
}

void UpdateFrontSensor(bool front_sensor_input)
{
    latest.front = front_sensor_input;
}

void UpdateLeftSensor(bool left_sensor_input)
{
    latest.left = left_sensor_input;
}

void UpdateRightSensor(bool right_sensor_input)
{
    latest.right = right_sensor_input;
}

void UpdateDustSensor(bool dust_sensor_input)
{
    latest.dust = dust_sensor_input;
}

SensorValues ReadLatestSensorValues(void)
{
    return latest;
}
