// System tests: whole-system scenarios on a 10x10 grid (tests/system/grid_world.h).
// The real C controller runs through the top module Main for at most 100 ticks.
// Each scenario checks several essential functions and the global invariants.
#include <gtest/gtest.h>
#include <algorithm>
#include "grid_world.h"

using namespace sys;

static const std::vector<std::string> EMPTY = {
    "..........", "..........", "..........", "..........", "..........",
    "..........", "..........", "..........", "..........", "..........",
};

static bool turning(const TickRecord &r) {
    return r.status.state == STATE_TURN_LEFT || r.status.state == STATE_TURN_RIGHT;
}

// Requirement invariants checked on every tick of every scenario.
static void check_invariants(const GridWorld &w, const std::vector<TickRecord> &tr) {
    ASSERT_LE(tr.size(), static_cast<size_t>(GridWorld::MAX_TICKS));
    for (size_t i = 0; i < tr.size(); ++i) {
        const TickRecord &r = tr[i];
        const ControllerStatus &s = r.status;
        SCOPED_TRACE("tick " + std::to_string(r.tick));
        // PR-13: clean_off only while backing up or turning; stop keeps cleaning.
        bool off_state = turning(r) || s.state == STATE_BACKWARD;
        EXPECT_EQ(s.clean_command == CLEAN_OFF, off_state);
        if (s.direction_command == DIR_STOP) {
            EXPECT_EQ(s.clean_command, CLEAN_UP);
        }
        // PR-2: in Forward the RVC goes forward while cleaning.
        if (s.state == STATE_FORWARD) {
            EXPECT_EQ(s.direction_command, DIR_FORWARD);
            EXPECT_EQ(s.clean_command, CLEAN_ON);
        }
        // PR-11: one clean_up activation is at most 3 ticks.
        EXPECT_LE(s.cleanup_count, 3);
        // PR-10: a turn lasts 3 ticks: counts go 1,2,3 and the turn is not cut short.
        if (turning(r)) {
            EXPECT_LE(s.turn_count, 3);
            if (i + 1 < tr.size() && s.turn_count < 3) {
                EXPECT_EQ(tr[i + 1].status.state, s.state);
                EXPECT_EQ(tr[i + 1].status.turn_count, s.turn_count + 1);
            }
        }
        // PR-9: a new turn picks left whenever the left side is clear.
        bool new_turn = turning(r) && s.turn_count == 1 && (i == 0 || !turning(tr[i - 1]) ||
                                                              tr[i - 1].status.turn_count == 3);
        if (new_turn && !r.sensors.left) {
            EXPECT_EQ(s.state, STATE_TURN_LEFT);
        }
        // The RVC never occupies an obstacle cell.
        EXPECT_FALSE(w.obstacle_at(r.x, r.y));
    }
}

static int first_index(const std::vector<TickRecord> &tr, ControllerState st, size_t from = 0) {
    for (size_t i = from; i < tr.size(); ++i)
        if (tr[i].status.state == st) return static_cast<int>(i);
    return -1;
}

// SYS-01 (PR-2, PR-3, PR-9, PR-10, PR-13): open room, straight to the wall,
// left turn has priority, turn lasts 3 ticks with clean_off, then forward again.
TEST(SystemScenario, SYS01_OpenRoomStraightThenLeftTurnAtWall) {
    GridWorld w(EMPTY, NORTH);
    auto tr = w.run(20);
    check_invariants(w, tr);
    for (int i = 0; i < 5; ++i) EXPECT_EQ(tr[i].status.state, STATE_FORWARD);
    EXPECT_EQ(tr[4].y, 1);
    for (int i = 5; i < 8; ++i) {
        EXPECT_EQ(tr[i].status.state, STATE_TURN_LEFT);
        EXPECT_EQ(tr[i].status.turn_count, i - 4);
        EXPECT_EQ(tr[i].status.clean_command, CLEAN_OFF);
        EXPECT_EQ(tr[i].y, 0);
    }
    EXPECT_EQ(tr[8].status.state, STATE_FORWARD);
    EXPECT_EQ(tr[8].heading, WEST);
}

// SYS-02 (PR-3, PR-9, PR-10, PR-13): left side blocked -> turn right, then forward.
TEST(SystemScenario, SYS02_LeftBlockedTurnsRight) {
    auto map = EMPTY;
    map[2][5] = '#';
    map[3][4] = '#';
    GridWorld w(map, NORTH);
    auto tr = w.run(10);
    check_invariants(w, tr);
    EXPECT_EQ(tr[2].status.state, STATE_TURN_RIGHT);
    EXPECT_EQ(tr[2].x, 5);
    EXPECT_EQ(tr[2].y, 3);
    EXPECT_EQ(tr[5].status.state, STATE_FORWARD);
    EXPECT_EQ(tr[5].heading, EAST);
}

static std::vector<std::string> dead_end() {
    auto map = EMPTY;
    map[1][5] = '#';
    for (int y = 2; y <= 5; ++y) map[y][4] = map[y][6] = '#';
    return map;
}

// SYS-03 (PR-4, PR-14, PR-9, PR-13): dead-end corridor -> backward until a side is
// clear, then a left turn (both sides clear), then forward.
TEST(SystemScenario, SYS03_DeadEndBacksUpThenTurnsLeft) {
    GridWorld w(dead_end(), NORTH);
    auto tr = w.run(20);
    check_invariants(w, tr);
    int b = first_index(tr, STATE_BACKWARD);
    ASSERT_GE(b, 0);
    EXPECT_EQ(tr[b].y, 2);
    int t = first_index(tr, STATE_TURN_LEFT, b);
    ASSERT_GT(t, b);
    for (int i = b; i < t; ++i) {
        EXPECT_EQ(tr[i].status.direction_command, DIR_BACKWARD);
        EXPECT_TRUE(tr[i].sensors.left && tr[i].sensors.right);  // both sides still blocked
    }
    EXPECT_FALSE(tr[t].sensors.left);
    EXPECT_FALSE(tr[t].sensors.right);  // both clear -> left priority
    EXPECT_EQ(tr[t].y, 6);
    EXPECT_EQ(tr[t + 3].status.state, STATE_FORWARD);
}

// SYS-04 (PR-5, PR-11, PR-2): long dust strip -> clean_up activations of at most
// 3 ticks restart while dust remains, then clean_on when the dust ends.
TEST(SystemScenario, SYS04_DustStripRestartsActivationEveryThreeTicks) {
    auto map = EMPTY;
    map[9][5] = 'R';
    for (int y = 3; y <= 7; ++y) map[y][5] = '1';
    GridWorld w(map, NORTH);
    auto tr = w.run(12);
    check_invariants(w, tr);
    std::vector<int> counts;
    for (int i = 2; i < 7; ++i) {
        EXPECT_EQ(tr[i].status.state, STATE_CLEANUP_FORWARD);
        EXPECT_EQ(tr[i].status.direction_command, DIR_FORWARD);
        counts.push_back(tr[i].status.cleanup_count);
    }
    EXPECT_EQ(counts, (std::vector<int>{1, 2, 3, 1, 2}));
    EXPECT_EQ(tr[7].status.state, STATE_FORWARD);
    EXPECT_EQ(tr[7].status.clean_command, CLEAN_ON);
}

// SYS-05 (PR-5, PR-11): a single dusty cell -> clean_up ends sooner when dust is gone.
TEST(SystemScenario, SYS05_ShortDustEndsActivationEarly) {
    auto map = EMPTY;
    map[3][5] = '2';
    GridWorld w(map, NORTH);
    auto tr = w.run(6);
    check_invariants(w, tr);
    EXPECT_EQ(tr[2].status.state, STATE_CLEANUP_FORWARD);
    EXPECT_EQ(tr[3].status.state, STATE_FORWARD);  // ended after 1 tick (< 3)
}

// SYS-06 (PR-15, PR-11, PR-3, PR-13): front obstacle during clean_up -> stop in
// place and keep cleaning until dust is gone (new activations while stopped),
// no turn or backward before that, then turn left.
TEST(SystemScenario, SYS06_FrontObstacleDuringCleanUpStopsUntilDustGone) {
    auto map = EMPTY;
    map[3][5] = '1';
    map[2][5] = '5';
    map[1][5] = '#';
    GridWorld w(map, NORTH);
    auto tr = w.run(15);
    check_invariants(w, tr);
    int s = first_index(tr, STATE_CLEANUP_STOPPED);
    ASSERT_GE(s, 0);
    int e = s;
    while (tr[e].status.state == STATE_CLEANUP_STOPPED) {
        EXPECT_EQ(tr[e].status.direction_command, DIR_STOP);
        EXPECT_EQ(tr[e].status.clean_command, CLEAN_UP);
        EXPECT_EQ(tr[e].x, 5);
        EXPECT_EQ(tr[e].y, 2);
        ++e;
    }
    EXPECT_EQ(e - s, 5);              // 5 units of dust cleaned while stopped
    EXPECT_FALSE(tr[e].sensors.dust);  // left the stop only when dust was gone
    EXPECT_EQ(tr[e].status.state, STATE_TURN_LEFT);
    EXPECT_EQ(w.dust_at(5, 2), 0);
}

// SYS-07 (PR-16, PR-14, PR-4, PR-11): dust while moving backward -> stop, one
// clean_up activation, then backward again, then a turn when a side clears.
TEST(SystemScenario, SYS07_DustWhileBackingUpPausesForOneActivation) {
    auto map = dead_end();
    map[4][5] = '2';
    GridWorld w(map, NORTH);
    auto tr = w.run(20);
    check_invariants(w, tr);
    int p = first_index(tr, STATE_CLEANUP_BACKWARD_PAUSE);
    ASSERT_GT(p, 0);
    EXPECT_EQ(tr[p - 1].status.state, STATE_BACKWARD);
    int e = p;
    while (tr[e].status.state == STATE_CLEANUP_BACKWARD_PAUSE) {
        EXPECT_EQ(tr[e].status.direction_command, DIR_STOP);
        EXPECT_EQ(tr[e].status.clean_command, CLEAN_UP);
        EXPECT_EQ(tr[e].y, tr[p].y);
        ++e;
    }
    EXPECT_LE(e - p, 3);
    EXPECT_EQ(tr[e].status.state, STATE_BACKWARD);  // backward again
    EXPECT_GE(first_index(tr, STATE_TURN_LEFT, e), 0);
}

// SYS-08 (PR-17, PR-10, PR-3, PR-5): dust found where a turn starts -> the turn
// finishes (3 ticks, clean_off) and the RVC then goes forward in clean_up.
TEST(SystemScenario, SYS08_DustDuringTurnFinishesTurnThenCleansUp) {
    auto map = EMPTY;
    map[0][5] = '1';
    GridWorld w(map, NORTH);
    auto tr = w.run(12);
    check_invariants(w, tr);
    for (int i = 5; i < 8; ++i) {
        EXPECT_EQ(tr[i].status.state, STATE_TURN_LEFT);
        EXPECT_TRUE(tr[i].status.dust_during_turn);
    }
    EXPECT_EQ(tr[8].status.state, STATE_CLEANUP_FORWARD);
    EXPECT_EQ(tr[8].status.direction_command, DIR_FORWARD);
    EXPECT_EQ(tr[8].status.clean_command, CLEAN_UP);
}

// SYS-09 (PR-3, PR-9, PR-10): corner start facing a wall with the left blocked
// -> immediate right turn on the first tick.
TEST(SystemScenario, SYS09_CornerStartTurnsRightImmediately) {
    auto map = EMPTY;
    map[5][5] = '.';
    map[0][0] = 'R';
    GridWorld w(map, NORTH);
    auto tr = w.run(8);
    check_invariants(w, tr);
    EXPECT_EQ(tr[0].status.state, STATE_TURN_RIGHT);
    EXPECT_EQ(tr[3].heading, EAST);
    EXPECT_EQ(tr[3].status.state, STATE_FORWARD);
}

// SYS-10 (PR-4, PR-14, PR-13): fully enclosed -> keeps backing up (clean_off)
// because no side ever becomes clear.
TEST(SystemScenario, SYS10_EnclosedKeepsBackingUp) {
    auto map = EMPTY;
    map[4][5] = map[6][5] = map[5][4] = map[5][6] = '#';
    GridWorld w(map, NORTH);
    auto tr = w.run(30);
    check_invariants(w, tr);
    for (const auto &r : tr) {
        EXPECT_EQ(r.status.state, STATE_BACKWARD);
        EXPECT_EQ(r.x, 5);
        EXPECT_EQ(r.y, 5);
    }
}

// SYS-11 (PR-8): the front sensor arrives by interrupt (only on change); the
// controller uses latest values, so the run equals one where front arrives every tick.
TEST(SystemScenario, SYS11_InterruptFrontEqualsEveryTickFront) {
    auto map = dead_end();
    map[3][5] = '3';
    GridWorld a(map, NORTH), b(map, NORTH);
    auto ta = a.run(40, true, 1);
    auto tb = b.run(40, false, 1);
    check_invariants(a, ta);
    ASSERT_EQ(ta.size(), tb.size());
    for (size_t i = 0; i < ta.size(); ++i) {
        EXPECT_EQ(ta[i].status.state, tb[i].status.state);
        EXPECT_EQ(ta[i].x, tb[i].x);
        EXPECT_EQ(ta[i].y, tb[i].y);
    }
}

static const std::vector<std::string> CLUTTERED = {
    "..#.......", ".....3..#.", "#...#.....", "..1....##.", "....#.....",
    ".#.......2", "...2.#....", "......#...", ".#.....1..", "....#.....",
};

// SYS-12 (PR-2, PR-3, PR-5, PR-10, PR-11, PR-13): 100-tick run in a cluttered room
// with dust; every requirement invariant holds and cleaning reaches several states.
TEST(SystemScenario, SYS12_ClutteredRoomHundredTicks) {
    GridWorld w(CLUTTERED, NORTH);
    auto tr = w.run(GridWorld::MAX_TICKS);
    check_invariants(w, tr);
    EXPECT_EQ(tr.size(), 100u);
    EXPECT_GE(first_index(tr, STATE_FORWARD), 0);
    EXPECT_GE(first_index(tr, STATE_TURN_LEFT), 0);
}

// SYS-13 (PR-8, PR-13, PR-11): periodic side and dust sensors sampled every 2 ticks;
// the controller keeps using the latest values and all invariants still hold.
TEST(SystemScenario, SYS13_SlowPeriodicSensorsKeepInvariants) {
    for (Heading h : {NORTH, EAST, SOUTH, WEST}) {
        GridWorld w(CLUTTERED, h);
        auto tr = w.run(GridWorld::MAX_TICKS, true, 2);
        check_invariants(w, tr);
    }
}

// SYS-14 (simulator rule): a scenario never runs past 100 ticks.
TEST(SystemScenario, SYS14_TickLimitIsHundred) {
    GridWorld w(EMPTY, SOUTH);
    auto tr = w.run(500);
    EXPECT_EQ(tr.size(), 100u);
    EXPECT_EQ(tr.back().tick, 100);
}
