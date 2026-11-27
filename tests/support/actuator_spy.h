// Test double for the motor/cleaner output port (records every command sent).
#ifndef ACTUATOR_SPY_H
#define ACTUATOR_SPY_H

#include <vector>
#include "actuator.h"

namespace spy {

struct Sent {
    char kind;  // 'D' = direction, 'C' = clean
    int value;
};

inline std::vector<Sent> &log() {
    static std::vector<Sent> entries;
    return entries;
}

inline void send_direction(Direction d) { log().push_back({'D', static_cast<int>(d)}); }
inline void send_clean(CleanMode c) { log().push_back({'C', static_cast<int>(c)}); }

inline const ActuatorPort *port() {
    static const ActuatorPort p = {send_direction, send_clean};
    return &p;
}

inline void reset() { log().clear(); }

// Last direction/clean sent, or -1 if none.
inline int last(char kind) {
    for (auto it = log().rbegin(); it != log().rend(); ++it) {
        if (it->kind == kind) return it->value;
    }
    return -1;
}

}  // namespace spy

#endif
