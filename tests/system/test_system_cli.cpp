// System tests of the delivered executable `rvc` (src/main.c + all modules):
// sensor samples go in on stdin, commands come out on stdout, one line per tick.
#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/wait.h>

#ifndef RVC_BIN
#error "RVC_BIN must point to the rvc executable"
#endif

struct Result {
    int exit_code;
    std::vector<std::string> lines;
};

static Result run_rvc(const std::string &input, const std::string &args = "") {
    static int n = 0;
    std::string in_path = "rvc_sys_input_" + std::to_string(n++) + ".txt";
    std::ofstream(in_path) << input;
    std::string cmd = std::string(RVC_BIN) + " " + args + " < " + in_path + " 2>/dev/null";
    FILE *p = popen(cmd.c_str(), "r");
    Result r{-1, {}};
    if (p == nullptr) return r;
    char buf[512];
    while (fgets(buf, sizeof buf, p) != nullptr) {
        std::string line(buf);
        if (!line.empty() && line.back() == '\n') line.pop_back();
        r.lines.push_back(line);
    }
    int status = pclose(p);
    r.exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    std::remove(in_path.c_str());
    return r;
}

static bool has(const std::string &line, const std::string &part) {
    return line.find(part) != std::string::npos;
}

// SYS-CLI-01 (PR-2, PR-3, PR-9, PR-10, PR-13): start forward/clean_on, front obstacle
// -> 3-tick left turn with clean_off -> forward.
TEST(SystemCli, CLI01_ObstacleAvoidanceSequence) {
    Result r = run_rvc("0 0 0 0\n1 0 0 0\n- - - -\n0 - - -\n- - - -\n");
    ASSERT_EQ(r.exit_code, 0);
    ASSERT_EQ(r.lines.size(), 6u);
    EXPECT_TRUE(has(r.lines[0], "tick=0 state=Forward dir=forward clean=clean_on"));
    EXPECT_TRUE(has(r.lines[1], "tick=1 state=Forward"));
    EXPECT_TRUE(has(r.lines[2], "tick=2 state=TurnLeft dir=turn_left clean=clean_off turn=1"));
    EXPECT_TRUE(has(r.lines[3], "turn=2"));
    EXPECT_TRUE(has(r.lines[4], "turn=3"));
    EXPECT_TRUE(has(r.lines[5], "tick=5 state=Forward dir=forward clean=clean_on"));
}

// SYS-CLI-02 (PR-5, PR-15, PR-11, PR-8): dust -> clean_up; front interrupt during
// clean_up -> stop; dust gone -> right turn (left blocked). Comments/blank lines skipped.
TEST(SystemCli, CLI02_CleanUpStopThenTurnRight) {
    Result r = run_rvc("# dust appears\n0 1 0 1\n\n1 - - -\n- - - 0\n");
    ASSERT_EQ(r.exit_code, 0);
    ASSERT_EQ(r.lines.size(), 4u);
    EXPECT_TRUE(has(r.lines[1], "state=CleanUpForward dir=forward clean=clean_up turn=0 cleanup=1"));
    EXPECT_TRUE(has(r.lines[2], "state=CleanUpStopped dir=stop clean=clean_up turn=0 cleanup=2"));
    EXPECT_TRUE(has(r.lines[3], "state=TurnRight dir=turn_right clean=clean_off"));
    EXPECT_TRUE(has(r.lines[3], "sensors=1100"));
}

// SYS-CLI-03 (PR-4, PR-16, PR-14): all blocked -> backward; dust -> stop + clean_up;
// one activation -> backward; right side clears -> turn right.
TEST(SystemCli, CLI03_BackwardDustPauseThenTurn) {
    Result r = run_rvc("1 1 1 0\n- - - 1\n- - - 1\n- - - 1\n- - - 1\n- - 0 0\n");
    ASSERT_EQ(r.exit_code, 0);
    ASSERT_EQ(r.lines.size(), 7u);
    EXPECT_TRUE(has(r.lines[1], "state=Backward dir=backward clean=clean_off"));
    EXPECT_TRUE(has(r.lines[2], "state=CleanUpBackwardPause dir=stop clean=clean_up turn=0 cleanup=1"));
    EXPECT_TRUE(has(r.lines[4], "cleanup=3"));
    EXPECT_TRUE(has(r.lines[5], "state=Backward"));  // one activation only, dust remains
    EXPECT_TRUE(has(r.lines[6], "state=TurnRight"));
}

// SYS-CLI-04: the optional tick limit stops the run.
TEST(SystemCli, CLI04_TickLimitArgument) {
    Result r = run_rvc("0 0 0 0\n0 0 0 0\n0 0 0 0\n0 0 0 0\n", "2");
    EXPECT_EQ(r.exit_code, 0);
    EXPECT_EQ(r.lines.size(), 3u);
}

// SYS-CLI-05: empty input -> only the start line.
TEST(SystemCli, CLI05_EmptyInput) {
    Result r = run_rvc("");
    EXPECT_EQ(r.exit_code, 0);
    ASSERT_EQ(r.lines.size(), 1u);
    EXPECT_TRUE(has(r.lines[0], "tick=0"));
}

// SYS-CLI-06: invalid input lines are rejected with exit code 2.
TEST(SystemCli, CLI06_InvalidInputRejected) {
    const char *bad[] = {"1 0 0\n",   "1 0 0 0 0\n", "1 0 x 0\n", "10 0 0 0\n",
                         "0 2 0 0\n", "0 0 0 01\n",  "\t \n0 0 - 7\n"};
    for (const char *in : bad) {
        Result r = run_rvc(in);
        EXPECT_EQ(r.exit_code, 2) << in;
        EXPECT_EQ(r.lines.size(), 1u) << in;  // only the start line was printed
    }
}

// SYS-CLI-07: an invalid tick-limit argument prints usage and exits with 2.
TEST(SystemCli, CLI07_InvalidArgument) {
    EXPECT_EQ(run_rvc("", "abc").exit_code, 2);
    EXPECT_EQ(run_rvc("", "5x").exit_code, 2);
}
