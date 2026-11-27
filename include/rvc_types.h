/* Shared data types of the RVC controller (artifacts/SA/data_dictionary.md). */
#ifndef RVC_TYPES_H
#define RVC_TYPES_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Direction Command: forward, backward, turn_left, turn_right, or stop. */
typedef enum {
    DIR_FORWARD = 0,
    DIR_BACKWARD,
    DIR_TURN_LEFT,
    DIR_TURN_RIGHT,
    DIR_STOP
} Direction;

/* Clean Command: clean_off, clean_on, or clean_up. */
typedef enum {
    CLEAN_OFF = 0,
    CLEAN_ON,
    CLEAN_UP
} CleanMode;

/* Latest Sensor Values (D1): four booleans, true means detected. */
typedef struct {
    bool front; /* Front Obstacle */
    bool left;  /* Left Obstacle  */
    bool right; /* Right Obstacle */
    bool dust;  /* Dust Detected  */
} SensorValues;

#ifdef __cplusplus
}
#endif

#endif /* RVC_TYPES_H */
