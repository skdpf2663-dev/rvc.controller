// Unit structural (white-box) tests for src/controller.c.
// Each test names the decision (line/function) it targets and the MC/DC
// independence pair it demonstrates: two runs that differ in exactly one
// condition and produce different decision outcomes.
#include "controller_fixture.h"

using S = ControllerFixture;
static constexpr bool T = true;
static constexpr bool _ = false;

// --- Controller(): (state==TurnLeft || state==TurnRight) && dust --------------
// Pair A (first condition): TurnLeft+dust sets the flag; Forward+dust does not start a turn flag.
// Pair B (second condition): TurnRight+dust sets; TurnRight+no dust keeps false.
TEST_F(S, DustDuringTurnFlag_TurnRightDustVsNoDust) {
    ControllerStatus s = tick(T, T, _, _);   // TurnRight, flag false
    EXPECT_FALSE(s.dust_during_turn);
    s = tick(T, T, _, _);                    // TurnRight, no dust -> stays false
    EXPECT_FALSE(s.dust_during_turn);
    s = tick(T, T, _, T);                    // TurnRight, dust -> true (2nd OR operand)
    EXPECT_TRUE(s.dust_during_turn);
}

TEST_F(S, DustDuringTurnFlag_NotSetOutsideTurnStates) {
    ControllerStatus s = tick(_, _, _, T);   // Forward + dust -> CleanUpForward
    EXPECT_FALSE(s.dust_during_turn);
    s = tick(_, _, _, T);                    // CleanUpForward + dust: not a turn state
    EXPECT_FALSE(s.dust_during_turn);
}

// --- step_forward: if (front) / else if (dust) ---------------------------------
TEST_F(S, StepForward_FrontTrueVsFalse) {
    EXPECT_EQ(tick(T, _, _, _).state, STATE_TURN_LEFT);  // front true
}
TEST_F(S, StepForward_DustTrueVsFalse) {
    EXPECT_EQ(tick(_, _, _, _).state, STATE_FORWARD);           // dust false
    EXPECT_EQ(tick(_, _, _, T).state, STATE_CLEANUP_FORWARD);   // dust true
}

// --- avoid_front_obstacle: !left / !right / else, from both callers ------------
TEST_F(S, Avoid_FromForward_AllThreeOutcomes) {
    EXPECT_EQ(tick(T, _, T, _).state, STATE_TURN_LEFT);
    SetUp();
    EXPECT_EQ(tick(T, T, _, _).state, STATE_TURN_RIGHT);
    SetUp();
    EXPECT_EQ(tick(T, T, T, _).state, STATE_BACKWARD);
}
TEST_F(S, Avoid_FromCleanUp_PassesFalseDustFlag) {
    tick(_, _, _, T);                            // CleanUpForward
    ControllerStatus s = tick(T, _, _, _);       // ended (no dust), front -> avoid
    EXPECT_EQ(s.state, STATE_TURN_LEFT);
    EXPECT_FALSE(s.dust_during_turn);
}

// --- start_turn / start_cleanup ternaries ---------------------------------------
TEST_F(S, StartTurn_LeftAndRightCommands) {
    EXPECT_EQ(tick(T, _, _, _).direction_command, DIR_TURN_LEFT);
    SetUp();
    EXPECT_EQ(tick(T, T, _, _).direction_command, DIR_TURN_RIGHT);
}
TEST_F(S, StartCleanup_ForwardAndStopCommands) {
    EXPECT_EQ(tick(_, _, _, T).direction_command, DIR_FORWARD);  // CleanUpForward
    SetUp();
    tick(T, T, T, _);
    EXPECT_EQ(tick(T, T, T, T).direction_command, DIR_STOP);     // CleanUpBackwardPause
}

// --- step_turn: turn_count < 3 / dust_during_turn / ternary on TurnRight --------
TEST_F(S, StepTurn_CountBelowLimitVsAtLimit) {
    tick(T, T, _, _);
    EXPECT_EQ(tick(T, T, _, _).direction_command, DIR_TURN_RIGHT);  // count 1 < 3
    tick(T, T, _, _);                                               // count 3
    EXPECT_EQ(tick(T, T, _, _).state, STATE_FORWARD);              // count == 3, no dust
}
TEST_F(S, StepTurn_DustFlagTrueVsFalseAtLimit) {
    tick(T, _, _, T);
    tick(_, _, _, _);
    tick(_, _, _, _);
    EXPECT_EQ(tick(_, _, _, _).state, STATE_CLEANUP_FORWARD);  // flag true
}

// --- step_backward: !left / !right / dust / else ---------------------------------
TEST_F(S, StepBackward_AllFourOutcomes) {
    const struct { bool l, r, d; ControllerState next; } rows[] = {
        {_, T, _, STATE_TURN_LEFT},
        {T, _, _, STATE_TURN_RIGHT},
        {T, T, T, STATE_CLEANUP_BACKWARD_PAUSE},
        {T, T, _, STATE_BACKWARD},
    };
    for (const auto &r : rows) {
        SetUp();
        tick(T, T, T, _);  // Backward
        EXPECT_EQ(tick(T, r.l, r.r, r.d).state, r.next);
    }
}

// --- step_backward_pause: cleanup_count == 3 || !dust ----------------------------
// MC/DC: (count<3, dust) -> stay; (count==3, dust) -> leave [cond 1]; (count<3, !dust) -> leave [cond 2].
TEST_F(S, BackwardPause_McdcPairs) {
    tick(T, T, T, _);
    tick(T, T, T, T);                                                // count 1
    EXPECT_EQ(tick(T, T, T, T).state, STATE_CLEANUP_BACKWARD_PAUSE); // F,F -> stay (count 2)
    tick(T, T, T, T);                                                // count 3
    EXPECT_EQ(tick(T, T, T, T).state, STATE_BACKWARD);               // T,F -> leave
    tick(T, T, T, T);                                                // new pause, count 1
    EXPECT_EQ(tick(T, T, T, _).state, STATE_BACKWARD);               // F,T -> leave
}

// --- step_cleanup: activation_ended = count == 3 || !dust -------------------------
TEST_F(S, Cleanup_ActivationEnded_McdcPairs) {
    tick(_, _, _, T);                                              // count 1
    EXPECT_EQ(tick(_, _, _, T).cleanup_count, 2);                  // F,F -> continues
    tick(_, _, _, T);                                              // count 3
    EXPECT_EQ(tick(_, _, _, T).cleanup_count, 1);                  // T,F -> ended, restarts
    EXPECT_EQ(tick(_, _, _, _).state, STATE_FORWARD);              // F,T -> ended, forward
}

// --- step_cleanup: is_forward && front (inside a continuing activation) ----------
TEST_F(S, Cleanup_Continuing_IsForwardAndFront_McdcPairs) {
    tick(_, _, _, T);
    EXPECT_EQ(tick(_, _, _, T).state, STATE_CLEANUP_FORWARD);   // forward, !front
    EXPECT_EQ(tick(T, _, _, T).state, STATE_CLEANUP_STOPPED);   // forward, front
    ControllerStatus s = tick(T, _, _, T);                      // !forward (stopped), front
    EXPECT_EQ(s.state, STATE_CLEANUP_STOPPED);
    EXPECT_EQ(s.direction_command, DIR_STOP);
}

// --- step_cleanup: next activation (!is_forward || front) ------------------------
TEST_F(S, Cleanup_NextActivation_McdcPairs) {
    // is_forward, !front -> forward activation
    tick(_, _, _, T); tick(_, _, _, T); tick(_, _, _, T);
    EXPECT_EQ(tick(_, _, _, T).state, STATE_CLEANUP_FORWARD);
    // is_forward, front -> stopped activation (second operand)
    SetUp();
    tick(_, _, _, T); tick(_, _, _, T); tick(_, _, _, T);
    EXPECT_EQ(tick(T, _, _, T).state, STATE_CLEANUP_STOPPED);
    // !is_forward, !front -> stopped activation (first operand)
    SetUp();
    tick(_, _, _, T); tick(T, _, _, T); tick(_, _, _, T);   // stopped, count 3
    EXPECT_EQ(tick(_, _, _, T).state, STATE_CLEANUP_STOPPED);
}

// --- step_cleanup: ended without dust, front true vs false -----------------------
TEST_F(S, Cleanup_EndedNoDust_FrontVsNoFront) {
    tick(_, _, _, T);
    tick(T, _, _, T);  // stopped
    EXPECT_EQ(tick(T, T, T, _).state, STATE_BACKWARD);
    SetUp();
    tick(_, _, _, T);
    tick(T, _, _, T);
    EXPECT_EQ(tick(_, T, T, _).state, STATE_FORWARD);
}

// The switch default in Controller() is defensive code. The state variable is
// file-static and only assigned the seven enum values, so no public input can
// reach it. It is reported as infeasible in reports/coverage-*.md.
