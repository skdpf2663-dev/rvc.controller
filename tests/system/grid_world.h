// System-test environment: a 10x10 grid world that turns map conditions into
// sensor samples and turns the controller's Direction commands into movement.
// The controller under test is the real C implementation, driven through the
// top module Main (RvcMain_Start / RvcMain_Cycle). The world model is test
// harness only; expected behavior is asserted from the requirements.
#ifndef GRID_WORLD_H
#define GRID_WORLD_H

#include <string>
#include <vector>
#include "controller.h"
#include "rvc_main.h"
#include "sensor_store.h"

namespace sys {

enum Heading { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };

struct TickRecord {
    int tick;
    int x, y;
    Heading heading;
    SensorValues sensors;  // latest values the controller used
    ControllerStatus status;
};

struct Direction2 { int dx, dy; };
inline Direction2 vec(Heading h) {
    static const Direction2 v[] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    return v[h];
}
inline Heading left_of(Heading h) { return static_cast<Heading>((h + 3) % 4); }
inline Heading right_of(Heading h) { return static_cast<Heading>((h + 1) % 4); }

class GridWorld {
public:
    static constexpr int N = 10;
    static constexpr int MAX_TICKS = 100;  // simulator rule: at most 100 ticks per scenario

    // Map rows: '.' empty, '#' obstacle, digit 1-9 dust amount, 'R' start (default center).
    explicit GridWorld(const std::vector<std::string> &rows, Heading h = NORTH)
        : heading_(h) {
        x_ = N / 2;
        y_ = N / 2;
        for (int y = 0; y < N; ++y)
            for (int x = 0; x < N; ++x) {
                char c = rows[y][x];
                obstacle_[y][x] = (c == '#');
                dust_[y][x] = (c >= '1' && c <= '9') ? c - '0' : 0;
                if (c == 'R') { x_ = x; y_ = y; }
            }
    }

    bool blocked(int x, int y) const {
        return x < 0 || y < 0 || x >= N || y >= N || obstacle_[y][x];
    }
    bool blocked_toward(Heading h) const {
        Direction2 d = vec(h);
        return blocked(x_ + d.dx, y_ + d.dy);
    }
    int dust_here() const { return dust_[y_][x_]; }
    int dust_at(int x, int y) const { return dust_[y][x]; }
    bool obstacle_at(int x, int y) const { return obstacle_[y][x]; }

    // front_on_change: the front sensor arrives by interrupt, so it is only
    // delivered when its value changes. side_period: left/right/dust sampling period.
    std::vector<TickRecord> run(int ticks = MAX_TICKS, bool front_on_change = true, int side_period = 1) {
        std::vector<TickRecord> trace;
        RvcMain_Start(nullptr);
        bool last_front = false;
        for (int t = 1; t <= ticks && t <= MAX_TICKS; ++t) {
            SensorSamples s = {};
            bool front = blocked_toward(heading_);
            if (!front_on_change || front != last_front || t == 1) {
                s.front_arrived = true;
                s.front = front;
                last_front = front;
            }
            if ((t - 1) % side_period == 0) {
                s.left_arrived = s.right_arrived = s.dust_arrived = true;
                s.left = blocked_toward(left_of(heading_));
                s.right = blocked_toward(right_of(heading_));
                s.dust = dust_here() > 0;
            }
            RvcMain_Cycle(&s);
            ControllerStatus st = Controller_GetStatus();
            trace.push_back({t, x_, y_, heading_, ReadLatestSensorValues(), st});
            apply(st);
        }
        return trace;
    }

private:
    void apply(const ControllerStatus &st) {
        if (st.clean_command == CLEAN_UP && dust_[y_][x_] > 0) dust_[y_][x_]--;  // power cleaning removes dust
        Direction2 d = vec(heading_);
        switch (st.direction_command) {
        case DIR_FORWARD:
            if (!blocked(x_ + d.dx, y_ + d.dy)) { x_ += d.dx; y_ += d.dy; }
            break;
        case DIR_BACKWARD:
            if (!blocked(x_ - d.dx, y_ - d.dy)) { x_ -= d.dx; y_ -= d.dy; }
            break;
        case DIR_TURN_LEFT:   // a turn takes 3 ticks; heading changes on the third
            if (st.turn_count == 3) heading_ = left_of(heading_);
            break;
        case DIR_TURN_RIGHT:
            if (st.turn_count == 3) heading_ = right_of(heading_);
            break;
        case DIR_STOP:
            break;
        }
    }

    int x_, y_;
    Heading heading_;
    bool obstacle_[N][N];
    int dust_[N][N];
};

}  // namespace sys

#endif
