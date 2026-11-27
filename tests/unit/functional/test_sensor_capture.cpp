// Unit functional (black-box) tests for SA Process 1, 1.1-1.4, 2.2.1.
// Source: process_specification.md (Process 1, 1.x, 2.2.1), data_dictionary.md (D1), PR-8.
#include <gtest/gtest.h>
#include "capture_sensor_values.h"
#include "sensor_store.h"

class SensorFunctional : public ::testing::Test {
protected:
    void SetUp() override { SensorStore_Reset(); }
};

static void expect_values(bool f, bool l, bool r, bool d) {
    SensorValues v = ReadLatestSensorValues();
    EXPECT_EQ(v.front, f);
    EXPECT_EQ(v.left, l);
    EXPECT_EQ(v.right, r);
    EXPECT_EQ(v.dust, d);
}

// UF-S01 (D1, FSM interpretation 2): every value is false until its first sample.
TEST_F(SensorFunctional, InitialValuesAreFalse) { expect_values(false, false, false, false); }

// UF-S02 (Process 1.1, PR-8): a front sample replaces only the front value.
TEST_F(SensorFunctional, UpdateFrontChangesOnlyFront) {
    UpdateFrontSensor(true);
    expect_values(true, false, false, false);
}

// UF-S03 (Process 1.2): a left sample replaces only the left value.
TEST_F(SensorFunctional, UpdateLeftChangesOnlyLeft) {
    UpdateLeftSensor(true);
    expect_values(false, true, false, false);
}

// UF-S04 (Process 1.3): a right sample replaces only the right value.
TEST_F(SensorFunctional, UpdateRightChangesOnlyRight) {
    UpdateRightSensor(true);
    expect_values(false, false, true, false);
}

// UF-S05 (Process 1.4): a dust sample replaces only the dust value.
TEST_F(SensorFunctional, UpdateDustChangesOnlyDust) {
    UpdateDustSensor(true);
    expect_values(false, false, false, true);
}

// UF-S06 (PR-8): the controller uses the latest value of each sensor.
TEST_F(SensorFunctional, LatestSampleWins) {
    UpdateFrontSensor(true);
    UpdateFrontSensor(false);
    UpdateDustSensor(false);
    UpdateDustSensor(true);
    expect_values(false, false, false, true);
}

// UF-S07 (Process 2.2.1): reading does not change the stored values.
TEST_F(SensorFunctional, ReadDoesNotModifyStore) {
    UpdateLeftSensor(true);
    UpdateRightSensor(true);
    (void)ReadLatestSensorValues();
    (void)ReadLatestSensorValues();
    expect_values(false, true, true, false);
}

// UF-S08 (Process 1): only sensors whose sample arrived are updated.
TEST_F(SensorFunctional, CaptureAppliesOnlyArrivedSamples) {
    UpdateLeftSensor(true);
    SensorSamples s = {};
    s.front_arrived = true;
    s.front = true;
    s.left_arrived = false;  // left keeps its previous value
    s.left = false;
    s.dust_arrived = true;
    s.dust = true;
    CaptureSensorValues(&s);
    expect_values(true, true, false, true);
}

// UF-S09 (Process 1): all four samples arriving together update all four values.
TEST_F(SensorFunctional, CaptureAppliesAllFourSamples) {
    SensorSamples s = {true, true, true, true, true, true, true, true};
    CaptureSensorValues(&s);
    expect_values(true, true, true, true);
    SensorSamples clear = {true, false, true, false, true, false, true, false};
    CaptureSensorValues(&clear);
    expect_values(false, false, false, false);
}

// UF-S10 (Process 1): servicing with no arrived sample keeps every value.
TEST_F(SensorFunctional, CaptureWithNothingArrivedKeepsValues) {
    UpdateRightSensor(true);
    SensorSamples none = {false, true, false, true, false, false, false, true};
    CaptureSensorValues(&none);
    CaptureSensorValues(nullptr);
    expect_values(false, false, true, false);
}
