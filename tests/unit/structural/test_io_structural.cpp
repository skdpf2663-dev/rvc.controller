// Unit structural (white-box) tests for actuator.c, capture_sensor_values.c, rvc_main.c.
#include <gtest/gtest.h>
#include <vector>
#include "actuator_spy.h"
#include "capture_sensor_values.h"
#include "generate_tick.h"
#include "rvc_main.h"
#include "sensor_store.h"

class IoStructural : public ::testing::Test {
protected:
    void SetUp() override {
        spy::reset();
        SensorStore_Reset();
    }
    void TearDown() override { Actuator_SetPort(nullptr); }
};

// actuator.c CommandDirection: port != NULL && port->send_direction != NULL
// MC/DC: (NULL, -) false [cond 1]; (port, NULL cb) false [cond 2]; (port, cb) true.
TEST_F(IoStructural, CommandDirection_McdcPairs) {
    Actuator_SetPort(nullptr);
    CommandDirection(DIR_STOP);
    EXPECT_TRUE(spy::log().empty());

    const ActuatorPort clean_only = {nullptr, spy::send_clean};
    Actuator_SetPort(&clean_only);
    CommandDirection(DIR_STOP);
    EXPECT_TRUE(spy::log().empty());

    Actuator_SetPort(spy::port());
    CommandDirection(DIR_STOP);
    EXPECT_EQ(spy::last('D'), DIR_STOP);
}

// actuator.c CommandCleanerMode: port != NULL && port->send_clean != NULL
TEST_F(IoStructural, CommandCleanerMode_McdcPairs) {
    Actuator_SetPort(nullptr);
    CommandCleanerMode(CLEAN_UP);
    EXPECT_TRUE(spy::log().empty());

    const ActuatorPort direction_only = {spy::send_direction, nullptr};
    Actuator_SetPort(&direction_only);
    CommandCleanerMode(CLEAN_UP);
    IssueActuatorCommands(DIR_BACKWARD, CLEAN_OFF);  // only the direction goes out
    ASSERT_EQ(spy::log().size(), 1u);
    EXPECT_EQ(spy::log()[0].kind, 'D');

    Actuator_SetPort(spy::port());
    CommandCleanerMode(CLEAN_UP);
    EXPECT_EQ(spy::last('C'), CLEAN_UP);
}

// capture_sensor_values.c: each "arrived" decision taken true and false independently.
TEST_F(IoStructural, Capture_EachArrivalDecisionBothWays) {
    for (int sensor = 0; sensor < 4; ++sensor) {
        SensorStore_Reset();
        SensorSamples s = {};
        bool *arrived[] = {&s.front_arrived, &s.left_arrived, &s.right_arrived, &s.dust_arrived};
        bool *value[] = {&s.front, &s.left, &s.right, &s.dust};
        *value[sensor] = true;
        CaptureSensorValues(&s);  // arrived false -> unchanged
        SensorValues v = ReadLatestSensorValues();
        EXPECT_FALSE(v.front || v.left || v.right || v.dust);
        *arrived[sensor] = true;
        CaptureSensorValues(&s);  // arrived true -> updated
        v = ReadLatestSensorValues();
        bool got[] = {v.front, v.left, v.right, v.dust};
        EXPECT_TRUE(got[sensor]);
    }
}

static bool never(SensorSamples *, void *) { return false; }
static int calls = 0;
static bool always(SensorSamples *s, void *) {
    ++calls;
    *s = SensorSamples{};
    return true;
}

// rvc_main.c RvcMain_Run loop: executed < max_ticks && source(...)
// MC/DC: (max 0) -> stop before calling the source [cond 1];
//        (max 5, source false) -> stop [cond 2]; (max 2, source true) -> run.
TEST_F(IoStructural, Run_LoopCondition_McdcPairs) {
    calls = 0;
    EXPECT_EQ(RvcMain_Run(spy::port(), always, nullptr, 0), 0UL);
    EXPECT_EQ(calls, 0);
    EXPECT_EQ(RvcMain_Run(spy::port(), never, nullptr, 5), 0UL);
    EXPECT_EQ(RvcMain_Run(spy::port(), always, nullptr, 2), 2UL);
    EXPECT_EQ(calls, 2);
    EXPECT_EQ(GenerateTick_Count(), 2UL);
}
