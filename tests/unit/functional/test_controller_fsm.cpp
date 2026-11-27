// Unit functional (black-box) tests for the Controller (SA Process 2.2.2).
// Each test is derived from one row of the transition table in
// artifacts/SA/DFD/dfd_level_4_fsm.md (FSM-Rxx) or from a requirement (PR-x).
#include "controller_fixture.h"

using F = ControllerFixture;

// ---- helpers that drive the FSM into a given state through the spec ----
static constexpr bool T = true;
static constexpr bool _ = false;

// FSM-R00: begin automatic cleaning -> Forward, forward, clean_on.
TEST_F(F, R00_InitialStateIsForwardCleanOn) {
    ControllerStatus s = Controller_GetStatus();
    expect(s, STATE_FORWARD, DIR_FORWARD, CLEAN_ON);
    EXPECT_EQ(s.turn_count, 0);
    EXPECT_EQ(s.cleanup_count, 0);
    EXPECT_FALSE(s.dust_during_turn);
}

// ---------------------------- Forward ----------------------------
// FSM-R01: Front and not Left -> TurnLeft (PR-3, PR-9 left priority).
TEST_F(F, R01_ForwardFrontLeftClearTurnsLeft) {
    ControllerStatus s = tick(T, _, _, _);
    expect(s, STATE_TURN_LEFT, DIR_TURN_LEFT, CLEAN_OFF);
    EXPECT_EQ(s.turn_count, 1);
    EXPECT_FALSE(s.dust_during_turn);
}

// FSM-R01 with dust: dust_during_turn receives the current Dust value.
TEST_F(F, R01_ForwardFrontWithDustRecordsDustDuringTurn) {
    ControllerStatus s = tick(T, _, T, T);
    expect(s, STATE_TURN_LEFT, DIR_TURN_LEFT, CLEAN_OFF);
    EXPECT_TRUE(s.dust_during_turn);
}

// FSM-R02: Front and Left and not Right -> TurnRight.
TEST_F(F, R02_ForwardFrontLeftBlockedTurnsRight) {
    ControllerStatus s = tick(T, T, _, _);
    expect(s, STATE_TURN_RIGHT, DIR_TURN_RIGHT, CLEAN_OFF);
    EXPECT_EQ(s.turn_count, 1);
}

// FSM-R03: Front and Left and Right -> Backward (PR-4).
TEST_F(F, R03_ForwardAllBlockedGoesBackward) {
    expect(tick(T, T, T, _), STATE_BACKWARD, DIR_BACKWARD, CLEAN_OFF);
}

// FSM-R04: not Front and Dust -> CleanUpForward (PR-5).
TEST_F(F, R04_ForwardDustPowersUpCleaning) {
    ControllerStatus s = tick(_, _, _, T);
    expect(s, STATE_CLEANUP_FORWARD, DIR_FORWARD, CLEAN_UP);
    EXPECT_EQ(s.cleanup_count, 1);
}

// FSM-R05: not Front and not Dust -> stay Forward (PR-2).
TEST_F(F, R05_ForwardClearKeepsGoing) {
    expect(tick(_, T, T, _), STATE_FORWARD, DIR_FORWARD, CLEAN_ON);
}

// Interpretation 5: front obstacle is handled before dust in Forward.
TEST_F(F, R01_FrontHasPriorityOverDustInForward) {
    expect(tick(T, _, _, T), STATE_TURN_LEFT, DIR_TURN_LEFT, CLEAN_OFF);
}

// ---------------------------- Turn ----------------------------
// FSM-R06 / PR-10: a turn lasts 3 ticks, then forward with clean_on (FSM-R08).
TEST_F(F, R06_R08_TurnLeftLastsThreeTicksThenForward) {
    tick(T, _, _, _);                                     // turn tick 1
    ControllerStatus s = tick(T, _, _, _);                // turn tick 2
    expect(s, STATE_TURN_LEFT, DIR_TURN_LEFT, CLEAN_OFF);
    EXPECT_EQ(s.turn_count, 2);
    s = tick(T, _, _, _);                                 // turn tick 3
    expect(s, STATE_TURN_LEFT, DIR_TURN_LEFT, CLEAN_OFF);
    EXPECT_EQ(s.turn_count, 3);
    s = tick(T, _, _, _);                                 // leaves, no 4th turn tick
    expect(s, STATE_FORWARD, DIR_FORWARD, CLEAN_ON);      // interpretation 7
    EXPECT_FALSE(s.dust_during_turn);
}

// FSM-R07 / PR-17: dust during a turn -> the turn finishes, then CleanUpForward.
TEST_F(F, R07_DustDuringTurnLeftFinishesTurnThenCleanUp) {
    tick(T, _, _, _);
    ControllerStatus s = tick(_, _, _, T);                // dust on turn tick 2
    expect(s, STATE_TURN_LEFT, DIR_TURN_LEFT, CLEAN_OFF);  // turn is not interrupted
    EXPECT_TRUE(s.dust_during_turn);
    tick(_, _, _, _);
    s = tick(_, _, _, _);                                 // dust gone at completion
    expect(s, STATE_CLEANUP_FORWARD, DIR_FORWARD, CLEAN_UP);
    EXPECT_EQ(s.cleanup_count, 1);
    EXPECT_FALSE(s.dust_during_turn);                     // cleared on leaving the turn
}

// Interpretation 8: dust seen only on the completion tick still selects clean_up.
TEST_F(F, R07_DustOnCompletionTickSelectsCleanUp) {
    tick(T, _, _, _);
    tick(_, _, _, _);
    tick(_, _, _, _);
    expect(tick(_, _, _, T), STATE_CLEANUP_FORWARD, DIR_FORWARD, CLEAN_UP);
}

// FSM-R09/R11: TurnRight lasts 3 ticks and returns to Forward.
TEST_F(F, R09_R11_TurnRightLastsThreeTicksThenForward) {
    tick(T, T, _, _);
    EXPECT_EQ(tick(T, T, _, _).turn_count, 2);
    ControllerStatus s = tick(T, T, _, _);
    expect(s, STATE_TURN_RIGHT, DIR_TURN_RIGHT, CLEAN_OFF);
    EXPECT_EQ(s.turn_count, 3);
    expect(tick(T, T, _, _), STATE_FORWARD, DIR_FORWARD, CLEAN_ON);
}

// FSM-R10: dust during TurnRight -> CleanUpForward after the turn.
TEST_F(F, R10_DustDuringTurnRightThenCleanUp) {
    tick(T, T, _, _);
    tick(T, T, _, T);
    tick(T, T, _, _);
    expect(tick(_, _, _, _), STATE_CLEANUP_FORWARD, DIR_FORWARD, CLEAN_UP);
}

// ---------------------------- Backward ----------------------------

// FSM-R12 / PR-14: Backward until left clears -> TurnLeft.
TEST_F(F, R12_BackwardLeftClearTurnsLeft) {
    tick(T, T, T, _);
    ControllerStatus s = tick(T, _, _, T);   // both clear: left priority, dust recorded
    expect(s, STATE_TURN_LEFT, DIR_TURN_LEFT, CLEAN_OFF);
    EXPECT_EQ(s.turn_count, 1);
    EXPECT_TRUE(s.dust_during_turn);         // interpretation 9
}

// FSM-R13: Backward with only right clear -> TurnRight.
TEST_F(F, R13_BackwardRightClearTurnsRight) {
    tick(T, T, T, _);
    expect(tick(T, T, _, _), STATE_TURN_RIGHT, DIR_TURN_RIGHT, CLEAN_OFF);
}

// FSM-R14 / PR-16: dust while moving backward -> stop and clean_up.
TEST_F(F, R14_BackwardDustStopsForCleanUp) {
    tick(T, T, T, _);
    ControllerStatus s = tick(T, T, T, T);
    expect(s, STATE_CLEANUP_BACKWARD_PAUSE, DIR_STOP, CLEAN_UP);
    EXPECT_EQ(s.cleanup_count, 1);
}

// FSM-R15 / PR-14: Backward continues while both sides are blocked.
TEST_F(F, R15_BackwardContinuesWhileSidesBlocked) {
    tick(T, T, T, _);
    expect(tick(_, T, T, _), STATE_BACKWARD, DIR_BACKWARD, CLEAN_OFF);
    expect(tick(T, T, T, _), STATE_BACKWARD, DIR_BACKWARD, CLEAN_OFF);
}

// ---------------------------- CleanUpBackwardPause ----------------------------
// FSM-R16/R17 / PR-11, PR-16: one activation of at most 3 ticks, then Backward again.
TEST_F(F, R16_R17_BackwardPauseIsOneActivationOfThreeTicks) {
    tick(T, T, T, _);
    tick(T, T, T, T);                                    // cleanup 1
    ControllerStatus s = tick(T, T, T, T);               // cleanup 2
    expect(s, STATE_CLEANUP_BACKWARD_PAUSE, DIR_STOP, CLEAN_UP);
    EXPECT_EQ(s.cleanup_count, 2);
    s = tick(T, T, T, T);                                // cleanup 3
    EXPECT_EQ(s.cleanup_count, 3);
    expect(tick(T, T, T, T), STATE_BACKWARD, DIR_BACKWARD, CLEAN_OFF);  // dust remains
}

// FSM-R17: the backward pause ends early when dust disappears.
TEST_F(F, R17_BackwardPauseEndsWhenDustGone) {
    tick(T, T, T, _);
    tick(T, T, T, T);
    expect(tick(T, T, T, _), STATE_BACKWARD, DIR_BACKWARD, CLEAN_OFF);
}

// Interpretation 10: a later Backward tick can start another single activation.
TEST_F(F, R14_LaterBackwardTickStartsAnotherActivation) {
    tick(T, T, T, _);
    tick(T, T, T, T);
    tick(T, T, T, T);
    tick(T, T, T, T);
    tick(T, T, T, T);  // back to Backward
    expect(tick(T, T, T, T), STATE_CLEANUP_BACKWARD_PAUSE, DIR_STOP, CLEAN_UP);
}

// ---------------------------- CleanUpForward ----------------------------
// FSM-R18 / PR-15: front obstacle during clean_up -> stop, same activation continues.
TEST_F(F, R18_CleanUpForwardFrontStopsSameActivation) {
    tick(_, _, _, T);
    ControllerStatus s = tick(T, _, _, T);
    expect(s, STATE_CLEANUP_STOPPED, DIR_STOP, CLEAN_UP);
    EXPECT_EQ(s.cleanup_count, 2);   // interpretation 11: count is not reset
}

// FSM-R19: activation continues with no front obstacle.
TEST_F(F, R19_CleanUpForwardContinues) {
    tick(_, _, _, T);
    ControllerStatus s = tick(_, _, _, T);
    expect(s, STATE_CLEANUP_FORWARD, DIR_FORWARD, CLEAN_UP);
    EXPECT_EQ(s.cleanup_count, 2);
}

// FSM-R20: activation ended (3 ticks), dust remains, front blocked -> new activation stopped.
TEST_F(F, R20_CleanUpForwardEndsDustRemainsFrontStops) {
    tick(_, _, _, T);
    tick(_, _, _, T);
    tick(_, _, _, T);
    ControllerStatus s = tick(T, _, _, T);
    expect(s, STATE_CLEANUP_STOPPED, DIR_STOP, CLEAN_UP);
    EXPECT_EQ(s.cleanup_count, 1);
}

// FSM-R21 / PR-11: clean_up never stays on longer than 3 ticks in one activation.
TEST_F(F, R21_CleanUpForwardRestartsActivationAfterThreeTicks) {
    tick(_, _, _, T);
    tick(_, _, _, T);
    EXPECT_EQ(tick(_, _, _, T).cleanup_count, 3);
    ControllerStatus s = tick(_, _, _, T);
    expect(s, STATE_CLEANUP_FORWARD, DIR_FORWARD, CLEAN_UP);
    EXPECT_EQ(s.cleanup_count, 1);
}

// FSM-R22: activation ended, no dust, front, left clear -> TurnLeft.
TEST_F(F, R22_CleanUpForwardEndsThenTurnLeft) {
    tick(_, _, _, T);
    ControllerStatus s = tick(T, _, _, _);
    expect(s, STATE_TURN_LEFT, DIR_TURN_LEFT, CLEAN_OFF);
    EXPECT_EQ(s.turn_count, 1);
    EXPECT_FALSE(s.dust_during_turn);
}

// FSM-R23: activation ended, no dust, front, left blocked, right clear -> TurnRight.
TEST_F(F, R23_CleanUpForwardEndsThenTurnRight) {
    tick(_, _, _, T);
    expect(tick(T, T, _, _), STATE_TURN_RIGHT, DIR_TURN_RIGHT, CLEAN_OFF);
}

// FSM-R24: activation ended, no dust, all blocked -> Backward.
TEST_F(F, R24_CleanUpForwardEndsThenBackward) {
    tick(_, _, _, T);
    expect(tick(T, T, T, _), STATE_BACKWARD, DIR_BACKWARD, CLEAN_OFF);
}

// FSM-R25 / PR-11: activation ends sooner when dust is no longer detected.
TEST_F(F, R25_CleanUpForwardEndsEarlyThenForward) {
    tick(_, _, _, T);
    expect(tick(_, T, T, _), STATE_FORWARD, DIR_FORWARD, CLEAN_ON);
}

// ---------------------------- CleanUpStopped ----------------------------

// FSM-R26: activation continues while stopped (even if the front clears, interp. 12).
TEST_F(F, R26_CleanUpStoppedContinues) {
    tick(_, _, _, T);
    tick(T, _, _, T);                       // stopped, count 2
    ControllerStatus s = tick(_, _, _, T);  // front cleared, still stopped
    expect(s, STATE_CLEANUP_STOPPED, DIR_STOP, CLEAN_UP);
    EXPECT_EQ(s.cleanup_count, 3);
}

// FSM-R27 / PR-15: activation ends with dust remaining -> next activation while stopped.
TEST_F(F, R27_CleanUpStoppedNextActivation) {
    tick(_, _, _, T);
    tick(T, _, _, T);
    tick(T, _, _, T);  // count 3
    ControllerStatus s = tick(_, _, _, T);
    expect(s, STATE_CLEANUP_STOPPED, DIR_STOP, CLEAN_UP);
    EXPECT_EQ(s.cleanup_count, 1);
}

// FSM-R28: dust gone, front, left clear -> TurnLeft.
TEST_F(F, R28_CleanUpStoppedThenTurnLeft) {
    tick(_, _, _, T);
    tick(T, _, _, T);
    expect(tick(T, _, T, _), STATE_TURN_LEFT, DIR_TURN_LEFT, CLEAN_OFF);
}

// FSM-R29: dust gone, front, left blocked, right clear -> TurnRight.
TEST_F(F, R29_CleanUpStoppedThenTurnRight) {
    tick(_, _, _, T);
    tick(T, _, _, T);
    expect(tick(T, T, _, _), STATE_TURN_RIGHT, DIR_TURN_RIGHT, CLEAN_OFF);
}

// FSM-R30: dust gone, all blocked -> Backward.
TEST_F(F, R30_CleanUpStoppedThenBackward) {
    tick(_, _, _, T);
    tick(T, _, _, T);
    expect(tick(T, T, T, _), STATE_BACKWARD, DIR_BACKWARD, CLEAN_OFF);
}

// FSM-R31: dust gone, front clear -> Forward.
TEST_F(F, R31_CleanUpStoppedThenForward) {
    tick(_, _, _, T);
    tick(T, _, _, T);
    expect(tick(_, _, _, _), STATE_FORWARD, DIR_FORWARD, CLEAN_ON);
}

// PR-13: clean_off is issued only in TurnLeft, TurnRight, and Backward.
TEST_F(F, PR13_CleanOffOnlyWhileTurningOrBackward) {
    const bool inputs[][4] = {
        {_, _, _, T}, {T, _, _, T}, {T, _, _, T}, {_, _, _, _}, {T, T, T, _}, {T, T, T, T},
        {T, T, T, T}, {T, T, T, _}, {T, T, _, T}, {_, _, _, _}, {_, _, _, _}, {_, _, _, _},
        {_, _, _, T}, {_, _, _, _}, {T, _, _, _}, {_, _, _, _}, {_, _, _, _}, {_, _, _, _},
    };
    for (const auto &in : inputs) {
        ControllerStatus s = tick(in[0], in[1], in[2], in[3]);
        bool off_state = s.state == STATE_TURN_LEFT || s.state == STATE_TURN_RIGHT ||
                         s.state == STATE_BACKWARD;
        EXPECT_EQ(s.clean_command == CLEAN_OFF, off_state);
        if (s.direction_command == DIR_STOP) {
            EXPECT_EQ(s.clean_command, CLEAN_UP);
        }
    }
}
