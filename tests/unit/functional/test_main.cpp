// Unit functional (black-box) tests for SD module Main (tick cycle of SA Process 0/2).
// Source: structure_chart.md section 3 (execution order on a tick) and section 5.
#include <gtest/gtest.h>
#include <vector>
#include "actuator_spy.h"
#include "controller.h"
#include "generate_tick.h"
#include "rvc_main.h"
#include "sensor_store.h"

class MainFunctional : public ::testing::Test {
protected:
    void SetUp() override { spy::reset(); }
    void TearDown() override { Actuator_SetPort(nullptr); }
};

// UF-M01: starting automatic cleaning issues forward and clean_on and clears old state.
TEST_F(MainFunctional, StartBeginsForwardCleanOn) {
    UpdateFrontSensor(true);  // leftover value from an earlier run
    RvcMain_Start(spy::port());
    ASSERT_EQ(spy::log().size(), 2u);
    EXPECT_EQ(spy::last('D'), DIR_FORWARD);
    EXPECT_EQ(spy::last('C'), CLEAN_ON);
    EXPECT_FALSE(ReadLatestSensorValues().front);
    EXPECT_EQ(GenerateTick_Count(), 0UL);
    EXPECT_EQ(Controller_GetStatus().state, STATE_FORWARD);
}

// UF-M02: one cycle = one Tick, samples applied before the decision, one controller execution.
TEST_F(MainFunctional, CycleCapturesBeforeDeciding) {
    RvcMain_Start(spy::port());
    spy::reset();
    SensorSamples s = {};
    s.front_arrived = true;
    s.front = true;
    RvcMain_Cycle(&s);
    EXPECT_EQ(GenerateTick_Count(), 1UL);
    EXPECT_EQ(Controller_GetStatus().state, STATE_TURN_LEFT);  // decision used the new sample
    ASSERT_EQ(spy::log().size(), 2u);                            // commands issued once
    EXPECT_EQ(spy::last('D'), DIR_TURN_LEFT);
}

// UF-M03: a cycle with no serviced sample still runs the controller on the latest values.
TEST_F(MainFunctional, CycleWithoutSamplesStillTicks) {
    RvcMain_Start(spy::port());
    UpdateDustSensor(true);
    RvcMain_Cycle(nullptr);
    EXPECT_EQ(GenerateTick_Count(), 1UL);
    EXPECT_EQ(Controller_GetStatus().state, STATE_CLEANUP_FORWARD);
}

struct Script {
    std::vector<SensorSamples> steps;
    size_t next = 0;
};

static bool script_source(SensorSamples *out, void *ctx) {
    Script *sc = static_cast<Script *>(ctx);
    if (sc->next >= sc->steps.size()) return false;
    *out = sc->steps[sc->next++];
    return true;
}

// UF-M04: Run executes one tick per input step and stops when input ends.
TEST_F(MainFunctional, RunStopsWhenInputEnds) {
    Script sc;
    sc.steps.resize(4);
    sc.steps[0].dust_arrived = true;
    sc.steps[0].dust = true;
    unsigned long n = RvcMain_Run(spy::port(), script_source, &sc, 100);
    EXPECT_EQ(n, 4UL);
    EXPECT_EQ(GenerateTick_Count(), 4UL);
    // forward/clean_on at start + 4 ticks x 2 commands
    EXPECT_EQ(spy::log().size(), 10u);
}

// UF-M05: Run never exceeds the tick limit.
TEST_F(MainFunctional, RunStopsAtTickLimit) {
    Script sc;
    sc.steps.resize(10);
    unsigned long n = RvcMain_Run(spy::port(), script_source, &sc, 3);
    EXPECT_EQ(n, 3UL);
    EXPECT_EQ(sc.next, 3u);
}

// UF-M06: Run without a source executes no tick (only the start commands).
TEST_F(MainFunctional, RunWithoutSourceDoesNothing) {
    EXPECT_EQ(RvcMain_Run(spy::port(), nullptr, nullptr, 5), 0UL);
    EXPECT_EQ(spy::log().size(), 2u);
}
