// Unit functional (black-box) tests for SA Process 2.1 and 3, 3.1, 3.2.
// Source: structure_chart.md (module interfaces), process_specification.md.
#include <gtest/gtest.h>
#include "actuator_spy.h"
#include "generate_tick.h"

// UF-T01 (Process 2.1): each GenerateTick call produces exactly one Tick.
TEST(TickFunctional, EachCallProducesOneTick) {
    GenerateTick_Reset();
    EXPECT_EQ(GenerateTick_Count(), 0UL);
    GenerateTick();
    GenerateTick();
    GenerateTick();
    EXPECT_EQ(GenerateTick_Count(), 3UL);
    GenerateTick_Reset();
    EXPECT_EQ(GenerateTick_Count(), 0UL);
}

class ActuatorFunctional : public ::testing::Test {
protected:
    void SetUp() override {
        spy::reset();
        Actuator_SetPort(spy::port());
    }
    void TearDown() override { Actuator_SetPort(nullptr); }
};

// UF-A01 (Process 3.1): CommandDirection sends the received Direction Command unchanged.
TEST_F(ActuatorFunctional, CommandDirectionSendsDirection) {
    const Direction all[] = {DIR_FORWARD, DIR_BACKWARD, DIR_TURN_LEFT, DIR_TURN_RIGHT, DIR_STOP};
    for (Direction d : all) {
        CommandDirection(d);
        EXPECT_EQ(spy::last('D'), static_cast<int>(d));
    }
    EXPECT_EQ(spy::last('C'), -1);
}

// UF-A02 (Process 3.2): CommandCleanerMode sends the received Clean Command unchanged.
TEST_F(ActuatorFunctional, CommandCleanerModeSendsClean) {
    const CleanMode all[] = {CLEAN_OFF, CLEAN_ON, CLEAN_UP};
    for (CleanMode c : all) {
        CommandCleanerMode(c);
        EXPECT_EQ(spy::last('C'), static_cast<int>(c));
    }
    EXPECT_EQ(spy::last('D'), -1);
}

// UF-A03 (Process 3, structure chart order): both commands, direction first.
TEST_F(ActuatorFunctional, IssueSendsDirectionThenClean) {
    IssueActuatorCommands(DIR_TURN_RIGHT, CLEAN_OFF);
    ASSERT_EQ(spy::log().size(), 2u);
    EXPECT_EQ(spy::log()[0].kind, 'D');
    EXPECT_EQ(spy::log()[0].value, DIR_TURN_RIGHT);
    EXPECT_EQ(spy::log()[1].kind, 'C');
    EXPECT_EQ(spy::log()[1].value, CLEAN_OFF);
}

// UF-A04 (c-coding rule: check pointers): with no port connected nothing is sent and nothing crashes.
TEST_F(ActuatorFunctional, NoPortDropsCommands) {
    Actuator_SetPort(nullptr);
    IssueActuatorCommands(DIR_FORWARD, CLEAN_ON);
    EXPECT_TRUE(spy::log().empty());
}
