#include "sensor_store.h"
void SensorStore_Reset(void) {}
void UpdateFrontSensor(bool v) { (void)v; }
void UpdateLeftSensor(bool v) { (void)v; }
void UpdateRightSensor(bool v) { (void)v; }
void UpdateDustSensor(bool v) { (void)v; }
SensorValues ReadLatestSensorValues(void) { SensorValues v = {false, false, false, false}; return v; }
