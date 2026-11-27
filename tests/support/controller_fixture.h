// Fixture that wires the Controller to the sensor store and the actuator spy.
#ifndef CONTROLLER_FIXTURE_H
#define CONTROLLER_FIXTURE_H

#include <gtest/gtest.h>
#include "actuator_spy.h"
#include "controller.h"
#include "sensor_store.h"

class ControllerFixture : public ::testing::Test {
protected:
    void SetUp() override {
        SensorStore_Reset();
        spy::reset();
        Actuator_SetPort(spy::port());
        Controller_Init();
    }
    void TearDown() override { Actuator_SetPort(nullptr); }

    // Sets the latest sensor values, then executes one controller tick.
    ControllerStatus tick(bool f, bool l, bool r, bool d) {
        UpdateFrontSensor(f);
        UpdateLeftSensor(l);
        UpdateRightSensor(r);
        UpdateDustSensor(d);
        Controller();
        return Controller_GetStatus();
    }

    static void expect(const ControllerStatus &s, ControllerState st, Direction dir, CleanMode clean) {
        EXPECT_EQ(s.state, st);
        EXPECT_EQ(s.direction_command, dir);
        EXPECT_EQ(s.clean_command, clean);
        // The command delivered to the hardware matches the selected command.
        EXPECT_EQ(spy::last('D'), static_cast<int>(dir));
        EXPECT_EQ(spy::last('C'), static_cast<int>(clean));
    }
};

#endif
